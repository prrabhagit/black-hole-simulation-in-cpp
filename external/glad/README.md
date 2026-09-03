Place your classic-GLAD output here (generated via https://glad.dav1d.de,
Language=C/C++, API gl=4.6, Profile=Core, "Generate a loader" checked):

  external/glad/include/glad/glad.h
  external/glad/include/KHR/khrplatform.h
  external/glad/src/glad.c

This matches the classic GLAD naming this project's source code expects
(#include <glad/glad.h>, GLADloadproc, gladLoadGLLoader). GLAD2's output
uses different names (glad/gl.h, GLADloadfunc) and is NOT drop-in
compatible without source changes.
