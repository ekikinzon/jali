#include <UnitTest++.h>

#include <iostream>

#include "../Mesh_MSTK.hh"

// Special class just for testing protected functions

namespace Jali {
class Mesh_MSTK_Test_Protected : public Mesh_MSTK {
 public:

  // Simplified constructor that generates a 3D mesh

  Mesh_MSTK_Test_Protected(double const x0, double const y0, double const z0,
                           double const x1, double const y1, double const z1,
                           int const nx, int const ny, int const nz,
                           MPI_Comm const& comm) :
      Mesh_MSTK(x0, y0, z0, x1, y1, z1, nx, ny, nz, comm) {}

  // Destructor

  ~Mesh_MSTK_Test_Protected() {}

  // function to test
  int block_partition_regular_mesh(int dim, double *domain,
                                   int *num_cells_in_dir,
                                   int num_blocks_requested,
                                   std::vector<std::array<double, 6>> *blocklimits,
                                   std::vector<std::array<int, 3>> *blocknumcells) {
    return Mesh_MSTK::block_partition_regular_mesh(dim, domain,
                                                   num_cells_in_dir,
                                                   num_blocks_requested,
                                                   blocklimits, blocknumcells);
  }

};  // end class Mesh_MSTK_Test_Protected
}  // end namespace Jali

int check_block_partitioning(int const dim, double const * const domain,
                             int const * const num_cells_in_dir,
                             int const num_blocks,
                             std::vector<std::array<double, 6>> const& blocklimits,
                             std::vector<std::array<int, 3>> const& blocknumcells) {

  if (blocklimits.size() != num_blocks) return 0;
  if (blocknumcells.size() != num_blocks) return 0;

  // Minimum sanity check - sum of volumes of blocks should be the
  // volume of the original domain - doesn't preclude blocks on top of
  // each other

  double total_volume = 1.0;
  for (int dir = 0; dir < dim; dir++)
    total_volume *= (domain[2*dir+1]-domain[2*dir]);
  double volume_sum = 0.0;
  for (int ib = 0; ib < num_blocks; ib++) {
    double block_volume = 1.0;
    for (int dir = 0; dir < dim; dir++)
      block_volume *= (blocklimits[ib][2*dir+1] - blocklimits[ib][2*dir]);
    volume_sum += block_volume;
  }
  double reldiff = fabs(volume_sum-total_volume)/total_volume;
  if (reldiff > 1.0e-09) {
    CHECK(reldiff > 1.0e-09); // so it prints a message
    return 0;
  }

  // Sum of the number of cells in blocks should be equal to each other
  int ncells = 1;
  for (int dir = 0; dir < dim; dir++)
    ncells *= num_cells_in_dir[dir];
  int ncells_sum = 0.0;
  for (int ib = 0; ib < num_blocks; ib++) {
    int ncells_block = 1;
    for (int dir = 0; dir < dim; dir++)
      ncells_block *= blocknumcells[ib][dir];
    ncells_sum += ncells_block;
  }
  if (ncells_sum != ncells) {
    CHECK_EQUAL(ncells_sum, ncells);  // so it prints out a message
    return 0;
  }



  // Extended check
  // Make sure that each block has neighbors on the interior side

  for (int ib = 0; ib < num_blocks; ib++) {
    for (int dir = 0; dir < dim; dir++) {
      for (int k = 0; k < 2; k++) {

        // If it is external boundary face, there will be no neighbors

        if (blocklimits[ib][2*dir+k] == domain[2*dir] ||
            blocklimits[ib][2*dir+k] == domain[2*dir+1]) continue;

        double block_face_coords[3][3];
        block_face_coords[0][0] = blocklimits[ib][2*dir+k];
        block_face_coords[0][1] = blocklimits[ib][2*((dir+1)%3)];
        block_face_coords[0][2] = blocklimits[ib][2*((dir+2)%3)];
        block_face_coords[1][0] = blocklimits[ib][2*dir+k];
        block_face_coords[1][1] = blocklimits[ib][2*((dir+1)%3)+1];
        block_face_coords[1][2] = blocklimits[ib][2*((dir+2)%3)+1];

        bool neighbor_found = false;
        for (int ib1 = 0; ib1 < num_blocks; ib1++) {
          if (ib == ib1) continue;

          double nbr_block_face_coords[3][3];
          nbr_block_face_coords[0][0] = blocklimits[ib][2*dir+k%2];
          nbr_block_face_coords[0][1] = blocklimits[ib][2*((dir+1)%3)];
          nbr_block_face_coords[0][2] = blocklimits[ib][2*((dir+2)%3)];
          nbr_block_face_coords[1][0] = blocklimits[ib][2*dir+k%2];
          nbr_block_face_coords[1][1] = blocklimits[ib][2*((dir+1)%3)+1];
          nbr_block_face_coords[1][2] = blocklimits[ib][2*((dir+2)%3)+1];

          if (block_face_coords[0][0] == nbr_block_face_coords[0][0] &&
              block_face_coords[0][1] == nbr_block_face_coords[0][1] &&
              block_face_coords[0][2] == nbr_block_face_coords[0][2] &&
              block_face_coords[1][0] == nbr_block_face_coords[1][0] &&
              block_face_coords[1][1] == nbr_block_face_coords[1][1] &&
              block_face_coords[1][2] == nbr_block_face_coords[1][2]) {
            neighbor_found = true;
            break;
          }
        }

        CHECK(neighbor_found);
        if (!neighbor_found) {
          std::cerr << "Could not find neighbor for block " << "[(" <<
              blocklimits[ib][0] << "," << blocklimits[ib][2] << "," <<
              blocklimits[ib][4] << "), (" <<
              blocklimits[ib][1] << "," << blocklimits[ib][3] << "," <<
              blocklimits[ib][5] << ")] across face [(" <<
              block_face_coords[0][0] << "," << block_face_coords[0][1] <<
              "," << block_face_coords[0][2] << "), (" <<
              block_face_coords[1][0] << "," << block_face_coords[1][1] <<
              "," << block_face_coords[1][2] << ")]\n";
        }
      }
    }
  }
}

TEST(BLOCK_PARTITION) {

  // Generate a mesh consisting of 3x3x3 elements or whatever -
  // doesn't matter since we won't get a partitioning based on the
  // actual mesh - its just a way to access the method

  Jali::Mesh_MSTK_Test_Protected *mesh =
      new Jali::Mesh_MSTK_Test_Protected(0.0, 0.0, 0.0, 1.0, 1.0, 1.0,
                                         3, 3, 3, MPI_COMM_WORLD);

  {
    // Partition a 1D domain
    int dim = 1;

    std::vector<std::array<double, 6>> blocklimits;
    std::vector<std::array<int, 3>> blocknumcells;

    // Domain is [0.0,1.0]
    double domain[6] = {0.0, 1.0, 0.0, 0.0, 0.0, 0.0};

    // 16 cells to force regular partition
    int num_cells_in_dir[3] = {16, 0, 0};

    int nparts = 4;

    int ok = mesh->block_partition_regular_mesh(dim, domain, num_cells_in_dir,
                                               nparts,
                                               &blocklimits, &blocknumcells);
    CHECK(ok);

    ok = check_block_partitioning(dim, domain, num_cells_in_dir, nparts,
                                  blocklimits, blocknumcells);
    CHECK(ok);

    // 15 cells to force irregular partition
    num_cells_in_dir[0] = 15;

    ok = mesh->block_partition_regular_mesh(dim, domain, num_cells_in_dir,
                                           nparts,
                                           &blocklimits, &blocknumcells);
    CHECK(ok);

    ok = check_block_partitioning(dim, domain, num_cells_in_dir, nparts,
                                  blocklimits, blocknumcells);
    CHECK(ok);
  }

  {
    // Partition a 2D domain
    int dim = 2;

    std::vector<std::array<double, 6>> blocklimits;
    std::vector<std::array<int, 3>> blocknumcells;

    // Domain is [0.0,1.0]x[0.0,1.0]
    double domain[6] = {0.0, 1.0, 0.0, 1.0, 0.0, 0.0};

    // 10 cells to force regular partition
    int num_cells_in_dir[3] = {10, 10, 0};

    int nparts = 4;

    int ok = mesh->block_partition_regular_mesh(dim, domain, num_cells_in_dir,
                                                nparts,
                                                &blocklimits, &blocknumcells);
    CHECK(ok);

    ok = check_block_partitioning(dim, domain, num_cells_in_dir, nparts,
                                  blocklimits, blocknumcells);
    CHECK(ok);

    // larger number of partitions to force irregular partitioning
    nparts = 16;

    ok = mesh->block_partition_regular_mesh(dim, domain, num_cells_in_dir,
                                            nparts,
                                            &blocklimits, &blocknumcells);
    CHECK(ok);

    ok = check_block_partitioning(dim, domain, num_cells_in_dir, nparts,
                                  blocklimits, blocknumcells);
    CHECK(ok);
  }

  {
    // Partition a 3D domain
    int dim = 3;

    std::vector<std::array<double, 6>> blocklimits;
    std::vector<std::array<int, 3>> blocknumcells;

    // Domain is [0.0,1.0]x[0.0,1.0]x[0.0,2.0]
    double domain[6] = {0.0, 1.0, 0.0, 1.0, 0.0, 2.0};

    // 10 cells to force regular partition
    int num_cells_in_dir[3] = {10, 10, 10};

    int nparts = 8;

    int ok = mesh->block_partition_regular_mesh(dim, domain, num_cells_in_dir,
                                                nparts,
                                                &blocklimits, &blocknumcells);
    CHECK(ok);

    ok = check_block_partitioning(dim, domain, num_cells_in_dir, nparts,
                                  blocklimits, blocknumcells);
    CHECK(ok);

    // More partitions and irregular cells to force irregular partition
    num_cells_in_dir[0] = 15;
    nparts = 32;

    ok = mesh->block_partition_regular_mesh(dim, domain, num_cells_in_dir,
                                            nparts,
                                            &blocklimits, &blocknumcells);
    CHECK(ok);

    ok = check_block_partitioning(dim, domain, num_cells_in_dir, nparts,
                                  blocklimits, blocknumcells);
    CHECK(ok);
  }

  delete(mesh);
}

