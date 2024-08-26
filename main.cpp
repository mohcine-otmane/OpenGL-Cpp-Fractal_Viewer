#include <windows.h>
#include <gl/gl.h>
#include <complex>
#include <iostream>
#include <vector>

/**************************
 * Function Declarations
 *
 **************************/

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void EnableOpenGL(HWND hWnd, HDC *hDC, HGLRC *hRC);
void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);
void DrawFractal(int width, int height, int max_iterations, double zoom, double offsetX, double offsetY, int formulaIndex);
void UpdateIterationControl(WPARAM wParam);
void UpdateZoomControl(WPARAM wParam, LPARAM lParam);
void CreateMenus(HWND hWnd);

/**************************
 * Global Variables
 *
 **************************/

int max_iterations = 10;  // Initial value for iterations
double zoom = 1.0;         // Zoom level
double offsetX = 0.0;      // X-axis offset for panning
double offsetY = 0.0;      // Y-axis offset for panning
int currentFormula = 0;    // Index of the selected formula

/**************************
 * WinMain
 *
 **************************/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow) {
    WNDCLASS wc;
    HWND hWnd;
    HDC hDC;
    HGLRC hRC;        
    MSG msg;
    BOOL bQuit = FALSE;

    /* register window class */
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "FractalVisualizer";
    RegisterClass(&wc);

    /* create main window */
    hWnd = CreateWindow(
        "FractalVisualizer", "Fractal Visualizer", 
        WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
        0, 0, 800, 800,
        NULL, NULL, hInstance, NULL
    );

    CreateMenus(hWnd);

    /* enable OpenGL for the window */
    EnableOpenGL(hWnd, &hDC, &hRC);

    /* program main loop */
    while (!bQuit) {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT) {
                bQuit = TRUE;
            } else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } else {
            /* OpenGL rendering code */
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            DrawFractal(800, 800, max_iterations, zoom, offsetX, offsetY, currentFormula);

            SwapBuffers(hDC);
            Sleep(1);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hWnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hWnd);

    return msg.wParam;
}

/**************************
 * Create Menus
 *
 **************************/

void CreateMenus(HWND hWnd) {
    HMENU hMenu = CreateMenu();
    HMENU hSubMenu = CreatePopupMenu();

    // Add more formulas here
    AppendMenu(hSubMenu, MF_STRING, 1, "Mandelbrot");
    AppendMenu(hSubMenu, MF_STRING, 2, "Julia");
    AppendMenu(hSubMenu, MF_STRING, 3, "Burning Ship");
    AppendMenu(hSubMenu, MF_STRING, 4, "Tricorn");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, "Select Formula");

    SetMenu(hWnd, hMenu);
}


/**************************
 * Draw Fractal
 *
 **************************/

void DrawFractal(int width, int height, int max_iterations, double zoom, double offsetX, double offsetY, int formulaIndex) {
    glBegin(GL_POINTS);
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            std::complex<float> point(
                (float)x / width * 3.5f / zoom - 2.5f / zoom + offsetX, 
                (float)y / height * 2.0f / zoom - 1.0f / zoom + offsetY
            );
            std::complex<float> z(0.0f, 0.0f);
            int iterations = 0;

            if (formulaIndex == 0) { // Mandelbrot
                while (abs(z) <= 2.0f && iterations < max_iterations) {
                    z = z * z + point;
                    ++iterations;
                }
            } else if (formulaIndex == 1) { // Julia
                std::complex<float> c(-0.7f, 0.27015f);
                while (abs(z) <= 2.0f && iterations < max_iterations) {
                    z = z * z + c;
                    ++iterations;
                }
            }

            float color = (float)iterations / max_iterations;
            
            // Apply color mapping based on the number of iterations
            if (iterations == max_iterations) {
                glColor3f(0.0f, 0.0f, 0.0f); // Inside the set (black)
            } else {
                // Smooth coloring
                glColor3f(0.5f + 0.5f * cos(3.0f + color * 6.28f),
                          0.5f + 0.5f * cos(2.0f + color * 6.28f),
                          0.5f + 0.5f * cos(1.0f + color * 6.28f));
            }

            glVertex2f((float)x / (width / 2) - 1.0f, (float)y / (height / 2) - 1.0f);
        }
    }
    glEnd();
}

/**************************
 * Window Procedure
 *
 **************************/

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        return 0;
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    case WM_DESTROY:
        return 0;
    case WM_KEYDOWN:
        UpdateIterationControl(wParam);
        return 0;
    case WM_MOUSEWHEEL:
        UpdateZoomControl(wParam, lParam);
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) { // Mandelbrot
            currentFormula = 0;
        } else if (LOWORD(wParam) == 2) { // Julia
            currentFormula = 1;
        }
        return 0;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

/**************************
 * Update Iteration Control
 *
 **************************/

void UpdateIterationControl(WPARAM wParam) {
    switch (wParam) {
        case VK_UP:
            max_iterations += 10;
            break;
        case VK_DOWN:
            max_iterations = (max_iterations > 10) ? max_iterations - 10 : 10; // Minimum iterations is 10
            break;
    }
    std::cout << "Max Iterations: " << max_iterations << std::endl;
}

/**************************
 * Update Zoom Control
 *
 **************************/

void UpdateZoomControl(WPARAM wParam, LPARAM lParam) {
    short delta = GET_WHEEL_DELTA_WPARAM(wParam);
    
    // Extract cursor position from lParam
    int mouseX = LOWORD(lParam); // LOWORD contains the X coordinate
    int mouseY = HIWORD(lParam); // HIWORD contains the Y coordinate

    // Convert mouse coordinates to normalized device coordinates
    double normalizedX = (double)mouseX / 800.0 * 3.5 - 2.5;
    double normalizedY = (double)mouseY / 800.0 * 2.0 - 1.0;

    // Adjust offsets based on zoom and mouse position
    if (delta > 0) {
        offsetX += normalizedX / zoom - normalizedX / (zoom * 1.1);
        offsetY += normalizedY / zoom - normalizedY / (zoom * 1.1);
        zoom *= 1.1;
    } else if (delta < 0) {
        offsetX += normalizedX / zoom - normalizedX / (zoom / 1.1);
        offsetY += normalizedY / zoom - normalizedY / (zoom / 1.1);
        zoom /= 1.1;
    }

    std::cout << "Zoom: " << zoom << ", OffsetX: " << offsetX << ", OffsetY: " << offsetY << std::endl;
}

/**************************
 * Enable OpenGL
 *
 **************************/

void EnableOpenGL(HWND hWnd, HDC *hDC, HGLRC *hRC) {
    PIXELFORMATDESCRIPTOR pfd;
    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hWnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    iFormat = ChoosePixelFormat(*hDC, &pfd);
    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);
    wglMakeCurrent(*hDC, *hRC);
}

/**************************
 * Disable OpenGL
 *
 **************************/

void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
}
