#include "graphicsScreen.h"
#include "StopWatch.h"
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include "sys/unistd.h"

#define WIDTH 800
#define HEIGHT 800
#define SIZE WIDTH*HEIGHT

#ifndef GRAPHICS
  #define gs_init(x,y)
  #define gs_plot(x,y,z)
  #define gs_exit()
  #define gs_update()
#endif

//The number of iterations decides how long we will compute before giving up
#define ITERATIONS 1024

#define MAX_HOSTNAME_LEN 256

#define WORKTAG 0
#define DIETAG 2

#define PACKAGE_Y 8 //8, 20
#define PACKAGE_X 8 //8, 20

void static_master(int num_proc, int rank);
void static_slave(int num_proc, int rank);
void dynamic_master(int num_proc, int rank);
void dynamic_slave(int num_proc, int rank);


int zooms=10;

int crc = 0;
int colorMap[HEIGHT][WIDTH];
int *palette;

double box_x_min, box_x_max, box_y_min, box_y_max;

/**
 * Translate from screen coordinates to space coordinates
 * 
 * @param x Screen coordinate
 * @returns Space coordinate
 */
inline double translate_x(int x) {       
  return (((box_x_max-box_x_min)/WIDTH)*x)+box_x_min;
}

/**
 * Translate from screen coordinates to space coordinates
 * 
 * @param y Screen coordinate
 * @returns Space coordinate
 */
inline double translate_y(int y) {
  return (((box_y_max-box_y_min)/HEIGHT)*y)+box_y_min;
}

/**
 * Mandelbrot divergence test
 * 
 * @param x,y Space coordinates
 * @returns Number of iterations before convergance
 */
inline int solve(double x, double y)
{
  double r=0.0,s=0.0;

  double next_r,next_s;
  int itt=0;
  
  while((r*r+s*s)<=4.0) {
    next_r=r*r-s*s+x;
    next_s=2*r*s+y;
    r=next_r; s=next_s;
    if(++itt==ITERATIONS)break;
  }
  
  return itt;
}

/** 
 * Set up the palette to translate from number of iterations to a color
 *
 */
void setup_colors() {
  palette = (int *)malloc(sizeof(int) * (ITERATIONS + 1));
  int temp_colors[4] = {0x0000ff, 0x00ff00, 0x00ff00, 0xff0000};
  int multiplier = 3;
  int i;
  for(i = 0; i <= ITERATIONS; i++) {
    float position = (float)(i/(float)(ITERATIONS + 1));
    float color_selection = position*multiplier;
    int real_selector = (int)floorf(color_selection);
    int real_color = 0x000000;
    real_color = (temp_colors[real_selector] + temp_colors[real_selector + 1] * color_selection - real_selector);
    palette[i] = real_color;
  }
  return;
}

/**
 * Creates all the mandelbrot images and shows them on screen
 * 
 */
void CreateMap(int num_proc, int rank) {
  //printf("############## CREATEMAP ###############\n");
  //call on master or slave
  //printf("RANK: %d and # process: %d\n", rank, num_proc);
  if (rank == 0)
  {
    static_master(num_proc, rank);
    //dynamic_master(num_proc, rank);
  }
  else
  {
    static_slave(num_proc, rank);
    //dynamic_slave(num_proc, rank);
  }
}


//original createmap
// void CreateMap() {
//   int x,y;

//   //Loops over pixels
//   for(y=0;y<HEIGHT;y++){
//     for(x=0;x<WIDTH;x++){

//       //The color is determined by the number of iterations
//       //More iterations means brighter color
//       colorMap[y][x]=palette[solve(translate_x(x),translate_y(y))];
//     }
//   }
//   for(x = 0; x < WIDTH; x++) {
//     for(y = 0; y < HEIGHT; y++) {
//       gs_plot(x, y, colorMap[y][x]);
//       crc += colorMap[y][x];
//     }
//   }
//   gs_update();
// }

/**
 * Sets up the coordinate space and generates the map at different zoom level
 * 
 */
void RoadMap (int num_proc, int rank)
{
  int i;
  double deltaxmin, deltaymin, deltaxmax,deltaymax;

  //Sets the bounding box, 
  //Note that the x_min is -1.5 instead of -2.5, since we are using a square window
  box_x_min=-1.5; box_x_max=0.5;
  box_y_min=-1.0; box_y_max=1.0;

  deltaxmin=(-0.9-box_x_min)/zooms;
  deltaxmax=(-0.65-box_x_max)/zooms;
  deltaymin=(-0.4-box_y_min)/zooms;
  deltaymax=(-0.1-box_y_max)/zooms;

  //Updates the map for every zoom level
  CreateMap(num_proc, rank);
  for(i=1;i<zooms;i++){
    box_x_min+=deltaxmin;
    box_x_max+=deltaxmax;
    box_y_min+=deltaymin;
    box_y_max+=deltaymax;
    CreateMap(num_proc, rank);
  }                       
}

/**
 * Sets up the coordinate space and generates the map at different zoom level
 * 
 */
// void RoadMap ()
// {
//   int i;
//   double deltaxmin, deltaymin, deltaxmax,deltaymax;

//   //Sets the bounding box, 
//   //Note that the x_min is -1.5 instead of -2.5, since we are using a square window
//   box_x_min=-1.5; box_x_max=0.5;
//   box_y_min=-1.0; box_y_max=1.0;

//   deltaxmin=(-0.9-box_x_min)/zooms;
//   deltaxmax=(-0.65-box_x_max)/zooms;
//   deltaymin=(-0.4-box_y_min)/zooms;
//   deltaymax=(-0.1-box_y_max)/zooms;

//   //Updates the map for every zoom level
//   CreateMap();
//   for(i=1;i<zooms;i++){
//     box_x_min+=deltaxmin;
//     box_x_max+=deltaxmax;
//     box_y_min+=deltaymin;
//     box_y_max+=deltaymax;
//     CreateMap();
//   }                       
// }


void dynamic_master(int num_proc, int rank)
{
  int i, y, x;
  /*Calculate package size and width where number of packets is either 8*8 or 20*20*/
  int package_height = 100; // 100, 40
  int package_width = 100; //100, 40
  int pkg_num= 0; //# pkg being sent
  int sent_pkg = (PACKAGE_Y * PACKAGE_X);
  int recv_pkg = 0;
  int count = 0;
  int pkg[(package_height*package_width)];
  MPI_Status status;

  /*send work/pkg-number to all slaves*/
  for (i = 1; i < num_proc; i++, pkg_num++)
  {
    MPI_Send(&pkg_num, 1, MPI_INT, i, WORKTAG, MPI_COMM_WORLD);
    sent_pkg--;
    count++;
  }
  
  /* As long as there is packets to receive, the master receive the packets and
     sends out new ones until it's not any packets left
   */
  do
  {
    MPI_Recv(&pkg, (package_height*package_width), MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    recv_pkg++;
    count--;

    /*Calculate where each packet should start end end depending on theyr tag. */
    int ys = ((status.MPI_TAG -(status.MPI_TAG % PACKAGE_Y))/PACKAGE_Y) * package_height;
    int ye = ys + package_height;
    int xs = ((status.MPI_TAG % PACKAGE_X) * package_width);
    int xe = xs + package_width;
    int pos = 0;
    
    for(y = ys; y < ye; y++)
    {
      for(x = xs; x < xe; x++, pos++)
      {
        //The color is determined by the number of iterations
        //More iterations means brighter color
        colorMap[y][x]= pkg[pos];
      } 
    }

    if (sent_pkg > 0)
    {
      /*Send more work to slaves */
      MPI_Send(&pkg_num, 1, MPI_INT, status.MPI_SOURCE, WORKTAG, MPI_COMM_WORLD);
      pkg_num++;
      sent_pkg--;
      count++;
    }

  } while(count > 0);

  /*Not any packets to send, send dietag to processes*/
  for(i = 1; i < num_proc; i++)
  {
    MPI_Send(&pkg_num, 1, MPI_INT, i, DIETAG, MPI_COMM_WORLD);
  }   

  for(x = 0; x < WIDTH; x++) {
    for(y = 0; y < HEIGHT; y++) {
      gs_plot(x, y, colorMap[y][x]);
      crc += colorMap[y][x];
    }
  }
  gs_update();
}


void dynamic_slave(int num_proc, int rank)
{
  int y, x;
  int package_height = 100; //100
  int package_width = 100; //100
  int pkg_num= 0; //pk being sent
  int pkg[(package_width*package_height)];
  MPI_Status status;

  /* Receive packet-number from master. 
     As long as the tag is a worktag, the slave should calculate
     where to start depending on its packet-number.
     THen the  slaves send back the packet with packetnumber as tag. 
  */
  MPI_Recv(&pkg_num, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

  while(status.MPI_TAG == WORKTAG) 
  {
    int ys = ((pkg_num -(pkg_num % PACKAGE_Y))/PACKAGE_Y) * package_height;
    int ye = ys + package_height;
    int xs = ((pkg_num % PACKAGE_X) * package_width);
    int xe = xs + package_width;
    int pos = 0;

    for(y = ys; y < ye; y++)
    {
      for(x = xs; x < xe; x++, pos++)
      {
        //The color is determined by the number of iterations
        //More iterations means brighter color
        pkg[pos] = palette[solve(translate_x(x),translate_y(y))];
      }
    }

    MPI_Send(&pkg, (package_width*package_height), MPI_INT, 0, pkg_num, MPI_COMM_WORLD); //send to master

    MPI_Recv(&pkg_num, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status); //recv next

    /*No more packets to be received, can just return to master*/
    if(status.MPI_TAG == DIETAG) {
      return;
    }
  }

}



void static_master(int num_proc, int rank) {
  int i, x, y;
  int rest = HEIGHT%(num_proc-1);
  int rows = (HEIGHT- rest)/(num_proc-1);

  int array[(rows*WIDTH)];
  int barray[((rows + rest)*WIDTH)];
  MPI_Status status;

  /*Receive result from outstanding work requests*/
  for(i = 1; i < num_proc; i++)
  {
    /* If number of processes is odd, one process must take a bigger amount
       of work. 
       It is therefore two different arrays with different calculations of how big they are,
       depending on the size of the row and rest.

    */
    if (i == (num_proc-1))
    {
      MPI_Recv(&barray, ((rows + rest) * WIDTH), MPI_INT, i, WORKTAG, MPI_COMM_WORLD, &status);

    }
    else 
    {
      MPI_Recv(&array, (rows * WIDTH), MPI_INT, i, WORKTAG, MPI_COMM_WORLD, &status);
    }

    int c = 0; 
    /*Put the two different arrays in colorMap and plot it on the screen*/
    if (i == (num_proc-1))
    {
      for(y = rows*(i-1); y < HEIGHT; y++) 
      {
        for(x = 0; x < WIDTH; x++, c++)
        {
          colorMap[y][x] = barray[c];
        }
      }
    }
    else
    {
      for(y = rows*(i-1); y < rows * i; y++) 
      {
        for(x = 0; x < WIDTH; x++, c++)
        {
          colorMap[y][x] = array[c];
        }
      }
    } 
  }

  for(x = 0; x < WIDTH; x++) {
    for(y = 0; y < HEIGHT; y++) {
      gs_plot(x, y, colorMap[y][x]);
      crc += colorMap[y][x];
    }
  }
  gs_update();
  

}


void static_slave(int num_proc, int rank) {
  int y, x, rows;
  
  int rest = HEIGHT%(num_proc-1);
  int max, size;
  
  /* If number of processes is odd, one process must take a bigger amount
   of work. 
   It is therefore two different arrays with different calculations of how big they are,
   depending on the size of the row and rest.

*/
  
  if (rank == (num_proc-1))
  {
    max = HEIGHT;
    rows = (HEIGHT-rest)/(num_proc-1);
    size = (rows + rest)*HEIGHT;
  }
  else
  {
    rows = (HEIGHT-rest)/(num_proc-1); //-1 for not master
    max = rows * rank;
    size = rows*HEIGHT;
  }
  
  int array[size];
  int pos = 0;
  
  for(y = rows*(rank-1); y < max; y++)
  {
    for(x = 0; x < WIDTH; x++, pos++)
    {

      //The color is determined by the number of iterations
      //More iterations means brighter color
      array[pos] = palette[solve(translate_x(x),translate_y(y))];
    }
  }
  
  MPI_Send(&array, size, MPI_INT, 0, WORKTAG, MPI_COMM_WORLD);
  
}


/**
 * Main function
 * 
 * @param argc, argv  Number of command-line arguments and the arguments
 * @returns 0 on success
 */
int main (int argc, char *argv[]){
  char buf[256];
  int num_proc, rank;


  if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
  {
    fprintf(stderr, "MPI initialization error\n");
    exit(EXIT_FAILURE);
  }


  MPI_Comm_size(MPI_COMM_WORLD, &num_proc); //Returns the size of the group associated with a communicator
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); //Determines the rank of the calling process in the communicator. 
  
  if (rank == 0)
  {
    gs_init(WIDTH, HEIGHT);
  }
  setup_colors();

  sw_init();
  sw_start();
  RoadMap(num_proc, rank);
  sw_stop();

  sw_timeString(buf);
  
  if (rank == 0)
  {
    printf("Time taken: %s\n",buf);

    printf("CRC is %x\n",crc);
    gs_exit();
  }

  
  

  MPI_Finalize();
  return 0;
}


/**
 * Main function
 * 
 * @param argc, argv  Number of command-line arguments and the arguments
 * @returns 0 on success
 */
// int main (int argc, char *argv[]){
//   char buf[256];
  
//   gs_init(WIDTH, HEIGHT);
//   setup_colors();

//   sw_init();
//   sw_start();
//   RoadMap();
//   sw_stop();

//   sw_timeString(buf);

//   printf("Time taken: %s\n",buf);

//   gs_exit();
//   printf("CRC is %x\n",crc);

//   return 0;
// } 



