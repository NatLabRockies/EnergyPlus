set(kiva_test_src
      foundation.unit.cpp
      domain_test.cpp
      cell_test.cpp
   )

add_executable(kiva_tests ${kiva_test_src})

target_link_libraries(kiva_tests PRIVATE gtest libkiva kiva_common_interface)

add_test(NAME unit.GC10aFixture.calculateADI COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=GC10aFixture.calculateADI")
add_test(NAME unit.GC10aFixture.calculateImplicit COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=GC10aFixture.calculateImplicit")
add_test(NAME unit.GC10aFixture.calculateCrankN COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=GC10aFixture.calculateCrankN")
add_test(NAME unit.GC10aFixture.calculateADE COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=GC10aFixture.calculateADE")
add_test(NAME unit.GC10aFixture.GC10a_calculateSteadyState COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=GC10aFixture.GC10a_calculateSteadyState")
add_test(NAME unit.GC10aFixture.calcCellADI COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=GC10aFixture.calcCellADI")
add_test(NAME unit.GC10aFixture.calcCellMatrix COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=GC10aFixture.calcCellMatrix")
add_test(NAME unit.GC10aFixture.calcCellMatrixSS COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=GC10aFixture.calcCellMatrixSS")

add_test(NAME unit.BESTEST.GC10a COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=BESTESTFixture.GC10a")
add_test(NAME unit.BESTEST.GC30a COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=BESTESTFixture.GC30a")
add_test(NAME unit.BESTEST.GC30b COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=BESTESTFixture.GC30b")
add_test(NAME unit.BESTEST.GC30c COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=BESTESTFixture.GC30c")
add_test(NAME unit.BESTEST.GC60b COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=BESTESTFixture.GC60b")
add_test(NAME unit.BESTEST.GC65b COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=BESTESTFixture.GC65b")
add_test(NAME unit.BESTEST.1D COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=BESTESTFixture.1D")

add_test(NAME unit.DomainFixture.domain_basics COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=DomainFixture.domain_basics")
add_test(NAME unit.DomainFixture.surface_indices COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=DomainFixture.surface_indices")
add_test(NAME unit.DomainFixture.surface_tilt COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=DomainFixture.surface_tilt")
add_test(NAME unit.DomainFixture.cell_vector COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=DomainFixture.cell_vector")

add_test(NAME unit.CellFixture.cell_basics COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=CellFixture.cell_basics")

add_test(NAME unit.AggregatorFixture.validation COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=AggregatorFixture.validation")

add_test(NAME unit.TypicalFixture.convectionCallback COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=TypicalFixture.convectionCallback")

add_test(NAME unit.FoundationFixture.foundationSurfaces COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=FoundationFixture.foundationSurfaces")
add_test(NAME unit.FoundationFixture.foundationSurfaces2 COMMAND $<TARGET_FILE:kiva_tests> "--gtest_filter=FoundationFixture.foundationSurfaces2")
