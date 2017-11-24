/*********************************************************
 *                                                       *
 * Frosty Trap                                           *
 *                                                       *
 * This is actually a classic SOR kernel                 *
 *                                                       *
 * Created by Brian Vinter, June 18th 1999               *
 * Minor updates, John Markus Bjørndalen, 2016-09-15     * 
 *                                                       *
 ********************************************************/

#include <omp.h>
#include <math.h>
#include "graphicsScreen.h"
#include <string.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <unistd.h>

#ifdef GRAPHICS
#else
#define gs_update gs_update_none
#define gs_exit   gs_update_none
#define gs_plot   gs_plot_none
#define gs_init   gs_init_none
void gs_update_none() {}
void gs_plot_none(int x, int y, int val) {}
void gs_init_none(int x, int y) {} 
#endif

//map we are building

#define HEIGHT 300
#define WIDTH 300
typedef double  arrtype[WIDTH][HEIGHT] ; 
arrtype trap_data;
arrtype trap_data2; 
int iters[HEIGHT];

int colortable[] = {
    WHITE,  // -273.15;-226.15
    BLUE,   // -226.15;-179.15
    CYAN,   // -179.15;-132.15
    YELLOW, // -132.15;- 85.15
    ORANGE, // - 85.15;- 38.15
    GREEN,  // - 38.15;+  8.85
    RED     // +  8.85;+ 55.85
};

#ifndef _OPENMP
int omp_get_max_threads() { return 0; } // for sequential version without openmp support
#endif

long long get_usecs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000LL + tv.tv_usec; 
}


int color(double data) 
{
    //We translate between -273.15 and 55.85
    data += 273.15;
    
    return colortable[(int)(data/47)];
}

void update()
{
#ifdef GRAPHICS
    int x,y;
    
    for (y = 1; y < HEIGHT - 1; y++)
	for (x = 1; x < WIDTH - 1; x++)
	    gs_plot(x, y, color(trap_data[x][y]));
    gs_update();
#endif
}

void update_2()
{
#ifdef GRAPHICS
    int x,y;
    
    for (y = 1; y < HEIGHT - 1; y++)
    for (x = 1; x < WIDTH - 1; x++)
        gs_plot(x, y, color(trap_data2[x][y]));
    gs_update();
#endif
}

inline double compute_new_pixel(arrtype *src, double omega, int x, int y)
{
    return (omega / 4.0) * ((*src)[x][y-1] +
                            (*src)[x][y+1] +
                            (*src)[x-1][y] + 
                            (*src)[x+1][y]) + 
           (1.0 - omega) * (*src)[x][y];
}

void CreateTrap() 
{    
    int x, y;

    // Set up the temperature at the sides.
    for (y = 0; y < HEIGHT; y++) {
        trap_data[       0][y] = -273.15;      // left side
        trap_data[ WIDTH-1][y] = -273.15;      // right side 
    }
    for (x = 0; x < WIDTH; x++) {
        trap_data[ x][0       ] =   40.0;      // top 
        trap_data[ x][HEIGHT-1] = -273.15;     // bottom
    }
}


/* Simple SOR algorithm. 
 */ 
int solve_simple()
{                                        
    int updatef = 0, show_freq = 100;
    int h = HEIGHT;
    int w = WIDTH;
    double epsilon = 0.001*h*w;
    double delta = epsilon;
    int x, y;
    double omega = 0.8; // 0.0 < omega < 2.0
    int total_iters = 0; 
    
    gs_update();
    while (delta >= epsilon) {
        delta = 0.0;
        #pragma omp parallel for private(x) reduction(+:delta)
        for (y = 1; y < h - 1; y++) {
            for (x = 1; x < w - 1; x++){
                double old = trap_data[x][y];
                double new = compute_new_pixel(&trap_data, omega, x, y); 
                trap_data[x][y] = new; 
                delta += fabs(old-new); 
            }
        }
        updatef += 1;
        if (updatef == show_freq){
            update();
            updatef = 0;
        }
        total_iters += 1; 
    }
    gs_update();
    return total_iters; 
}

/* Simple SOR algorithm, red black scheme
 */
int solve_rb() 
{                                        
    int updatef = 0, show_freq = 100;
    int h = HEIGHT;
    int w = WIDTH;
    double epsilon = 0.001*h*w;
    double delta = epsilon;
    int x, y;
    double omega = 0.8; // 0.0 < omega < 2.0
    int total_iters = 0;

    gs_update();
    while (delta >= epsilon)
    {
        delta = 0.0;
        /*Compute red points*/
        #pragma omp parallel for private(x) reduction(+: delta)
        for (y = 1; y < h - 1; y++)
         {
            for (x = 1; x < w -1; x++)
            {
                if ((y + x) % 2 != 0)
                {
                    double old = trap_data[x][y];
                    double new = compute_new_pixel(&trap_data, omega, x, y); 
                    trap_data[x][y] = new; 
                    delta += fabs(old-new);    
                }
            }
         }
        
        /*Compute black points*/
        #pragma omp parallel for private(x) reduction(+: delta)
        for (y = 1; y < h - 1; y++)
         {
            for (x = 1; x < w -1; x++)
            {
                if ((y + x) % 2 == 0)
                {
                    double old = trap_data[x][y];
                    double new = compute_new_pixel(&trap_data, omega, x, y); 
                    trap_data[x][y] = new; 
                    delta += fabs(old-new);    
                }
            }
         }

        updatef += 1;
        if (updatef == show_freq){
            update();
            updatef = 0;
        }
        total_iters += 1; 
     }


    gs_update();
    return total_iters;
}

/* Double buffering
 */ 
int solve_dbuf()                                  
{
    int updatef = 0, show_freq = 100;
    int h = HEIGHT;
    int w = WIDTH;
    double epsilon = 0.001*h*w;
    double delta = epsilon;
    int x, y;
    double omega = 0.8; // 0.0 < omega < 2.0
    int total_iters = 0;


   gs_update();
    while (delta >= epsilon)
    {
        delta = 0.0;
        #pragma omp parallel for private(x) reduction(+: delta)
        for (x = 1; x < w -1; x++)
        {
            for (y = 1; y < h - 1; y++)
            {
                /*Divide into odd and even numbers and use the previous (other data-store)
                  to calculate next point */
                if (total_iters % 2 != 0)
                {
                    double old = trap_data[x][y];
                    double new = compute_new_pixel(&trap_data2, omega, x, y); 
                    trap_data[x][y] = new; 
                    delta += fabs(old-new);    
                }
                else
                {
                    double old = trap_data2[x][y];
                    double new = compute_new_pixel(&trap_data, omega, x, y); 
                    trap_data2[x][y] = new; 
                    delta += fabs(old-new);    
                }
            }
         }

        updatef += 1;
        if (updatef == show_freq)
        {
            if (total_iters % 2 != 0)
            {
                update();
                updatef = 0;
            }
            else
            {
                update_2(); 
                updatef = 0;  
            }
        }
        total_iters += 1; 
     }
    
    gs_update();
    return total_iters;
}


int main(int argc, char **argv) 
{
    char *scheme = NULL;
    int (*cur_solver)() = NULL;
    char hostname[256]; 

    gethostname(hostname, 256); 
    
    XInitThreads();
    gs_init(WIDTH,HEIGHT);
    
    CreateTrap();

    // Find out which algorithm to use
    switch (argc) {
    case 2:
        if (strcmp("simple", argv[1]) == 0) {
            cur_solver = solve_simple; 
        } else if  (strcmp("dbuf", argv[1]) == 0) {
            cur_solver = solve_dbuf;
        } else if  (strcmp("rb", argv[1]) == 0) {
            cur_solver = solve_rb; 
        } else {
            printf("Can't figure out which you want. Assuming simple.\n");
            cur_solver = solve_simple; 
        }
        scheme = argv[1]; 
        break;
    default:
        scheme = "simple"; 
        cur_solver = solve_simple; 
    }   

    
    long long t1 = get_usecs();
    int total_iters = cur_solver(); 
    long long t2 = get_usecs();
    
    // Dump something that can be parsed as python/json. 
    printf("{ 'host': '%s', 'usecs': %lld, 'secs' : %f, 'scheme' : '%s', 'max_threads' : %d, 'total_iters' : %d}\n", 
           hostname, t2-t1, (t2-t1)/1000000.0, scheme, omp_get_max_threads(), total_iters);
    
    gs_exit();
    //while(1);
    return 0;
}
