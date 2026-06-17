/* Copyright (c) 2012-2022 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#include "fixtures/bestest-fixture.hpp"
#include "fixtures/typical-fixture.hpp"

#include "Errors.hpp"

using namespace Kiva;

static const double PI = 4.0 * atan(1.0);

class DomainFixture : public BESTESTFixture {
protected:
  void SetUp() {
    specifySystem();
    ground = std::make_shared<Ground>(fnd, outputMap);
    fnd.createMeshData();
    domain = std::make_shared<Domain>(ground->domain);
    domain->setDomain(fnd);
  };

  std::shared_ptr<Domain> domain;
};

TEST_F(DomainFixture, domain_basics) {
  EXPECT_EQ(domain->dim_lengths[0], 41u);
  EXPECT_EQ(domain->dim_lengths[1], 1u);
  EXPECT_EQ(domain->dim_lengths[2], 19u);
  EXPECT_EQ(domain->stepsize[0], 1u);
  EXPECT_EQ(domain->stepsize[1], 41u);
  EXPECT_EQ(domain->stepsize[2], 41u);

  EXPECT_EQ(domain->dest_index_vector.size(), 3u);
  EXPECT_EQ(domain->dest_index_vector[2].size(),
            domain->dim_lengths[0] * domain->dim_lengths[1] * domain->dim_lengths[2]);
}

TEST_F(DomainFixture, surface_indices) {
  EXPECT_EQ(ground->foundation.surfaces[0].indices.size(), domain->dim_lengths[2]);
  EXPECT_EQ(ground->foundation.surfaces[4].indices.size(), domain->dim_lengths[0]);
  EXPECT_EQ(ground->foundation.surfaces[5].indices.size(), 11u);
}

TEST_F(DomainFixture, surface_tilt) {
  EXPECT_DOUBLE_EQ(ground->foundation.surfaces[0].tilt, PI / 2);
  EXPECT_DOUBLE_EQ(ground->foundation.surfaces[4].tilt, PI);
  EXPECT_DOUBLE_EQ(ground->foundation.surfaces[5].tilt, 0.0);
}

TEST_F(DomainFixture, cell_vector) {
  EXPECT_EQ(domain->cell.size(),
            domain->dim_lengths[0] * domain->dim_lengths[1] * domain->dim_lengths[2]);

  EXPECT_EQ(domain->cell[0]->cellType, CellType::BOUNDARY);
  EXPECT_EQ(domain->cell[49]->cellType, CellType::NORMAL);
}
