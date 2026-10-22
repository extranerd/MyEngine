# CMakeList.txt: progetto CMake per MyEngine. Includere l'origine e definire
# qui la logica specifica del progetto.
#
find_package(SDL3 REQUIRED)
# Aggiungere l'origine all'eseguibile di questo progetto.
add_executable (MyEngine "MyEngine.cpp" "MyEngine.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MyEngine PROPERTY CXX_STANDARD 20)
endif()

# TODO: Aggiungere i test e, se necessario, installare le destinazioni.
target_link_libraries(MyEngine PRIVATE SDL3::SDL3)