rem *******************************************************************
rem *** Make sure to use the Visual Studio Developer Command Prompt ***
rem *******************************************************************
cl /EHsc /I. /Iinclude PhongRenderTest.cpp PhongRender.cpp lib/SphereMesh.cpp lib/*.lib opengl32.lib /link /subsystem:console

