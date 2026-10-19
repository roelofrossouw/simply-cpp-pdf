# Resources for Tests #
set(RESOURCE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resource")
set(RESOURCE_DESTINATION_DIR "${CMAKE_CURRENT_BINARY_DIR}/resource")
add_custom_target(copy_sc_pdf_resources ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${RESOURCE_SOURCE_DIR} ${RESOURCE_DESTINATION_DIR})

## TESTS ##
set(SC_TEST_LINK_LIBRARIES sc-pdf)

add_sc_test(pdf LABELS unit)
add_dependencies(test-pdf copy_sc_pdf_resources)

# sc-pdf's own writer: engine.cpp writes its PDFs into engine-output, then each checker that's
# installed (qpdf, pdfcpu, mutool) reads every one of them (check_pdfs.cmake); one that isn't is
# left out, as on a server without it.
find_package(ZLIB REQUIRED)
set(engine_output "${CMAKE_CURRENT_BINARY_DIR}/engine-output")
add_sc_test(engine LABELS unit LINK_LIBRARIES sc-pdf ZLIB::ZLIB)
target_include_directories(test-engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
set_tests_properties(test-engine PROPERTIES
        ENVIRONMENT "SC_PDF_ENGINE_OUTPUT=${engine_output}"
        FIXTURES_SETUP pdf_engine_files)
foreach (checker IN ITEMS qpdf pdfcpu mutool)
    find_program(${checker}_program ${checker})
    if (${checker}_program)
        add_test(NAME check-${checker}
                COMMAND ${CMAKE_COMMAND} -DCHECKER=${checker} -DTOOL=${${checker}_program} -DFOLDER=${engine_output}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/check_pdfs.cmake)
        set_tests_properties(check-${checker} PROPERTIES LABELS compliance FIXTURES_REQUIRED pdf_engine_files)
    else ()
        message(STATUS "${checker} not found: the PDFs aren't checked with it")
    endif ()
endforeach ()
