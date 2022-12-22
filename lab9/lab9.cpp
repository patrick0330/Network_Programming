#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;

#define	UNIXSTR_PATH "/sudoku.sock"	
#define DIM 9
#define BLANK 0
#define NEW_ROW "-------------------------------------"
#define GRID_FULL make_pair(9, 9)

void print_grid(int grid[DIM][DIM])
{
	for (int i = 0; i < DIM; i++)
	{
        fprintf(stdout, "    \n");
		fprintf(stdout, "%s\n", NEW_ROW);

		for (int j = 0; j < DIM; j++)
		{
			fprintf(stdout, " ");
			if (BLANK == grid[i][j])
			{
				fprintf(stdout, " ");
			}
			else
			{
                fprintf(stdout, "%d", grid[i][j]);				
			}
			fprintf(stdout, " ");
			fprintf(stdout, "|");
		}
	}
    fprintf(stdout, "\n%s\n\n", NEW_ROW);

}

bool used_in_row(int grid[DIM][DIM], int row, int num)
{
	for (int col = 0; col < DIM; col++)
		if (grid[row][col] == num)
		{
			return true;
		}
	return false;
}

bool used_in_col(int grid[DIM][DIM], int col, int num)
{
	for (int row = 0; row < DIM; row++)
		if (grid[row][col] == num)
		{
			return true;
		}
	return false;
}

bool used_in_box(int grid[DIM][DIM], int box_start_rpw, int box_start_col, int num)
{
	for (int row = 0; row < 3; row++)
		for (int col = 0; col < 3; col++)
			if (grid[row + box_start_rpw][col + box_start_col] == num) 
			{
				return true;
			}
	return false;
}

bool is_safe(int grid[DIM][DIM], int row, int col, int num)
{
	// Check if 'num' is not already placed in current row,
	// current column and current 3x3 box 
	return !used_in_row(grid, row, num) &&
		!used_in_col(grid, col, num) &&
		!used_in_box(grid, row - row % 3, col - col % 3, num);
}

pair<int, int> get_unassigned_location(int grid[DIM][DIM])
{
	for (int row = 0; row < DIM; row++)
		for (int col = 0; col < DIM; col++)
			if (grid[row][col] == BLANK)
			{
				return make_pair(row, col);
			}
	return GRID_FULL;
}

bool solve_soduko(int grid[DIM][DIM])
{
	// If the Soduko grid has been filled, we are done
	if (GRID_FULL == get_unassigned_location(grid))
	{
		return true; 
	}

	// Get an unassigned Soduko grid location
	pair<int, int> row_and_col = get_unassigned_location(grid);
	int row = row_and_col.first;
	int col = row_and_col.second;

	// Consider digits 1 to 9
	for (int num = 1; num <= 9; num++)
	{
		// If placing the current number in the current
		// unassigned location is valid, go ahead
		if (is_safe(grid, row, col, num))
		{
			// Make tentative assignment
			grid[row][col] = num;

			if (solve_soduko(grid))
			{
				return true;
			}

			grid[row][col] = BLANK;
		}
	}

	// If we have gone through all possible numbers for the current unassigned
	// location, then we probably assigned a bad number early. Lets backtrack 
	// and try a different number for the previous unassigned locations.
	return false; 
}



int main(int argc, char **argv) {

    int grid[DIM][DIM] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
						   { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
						   { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
						   { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
						   { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
						   { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
						   { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
						   { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
						   { 0, 0, 0, 0, 0, 0, 0, 0, 0 } };


    int sockfd;
    struct sockaddr_un servaddr;
    sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, UNIXSTR_PATH);
    connect(sockfd, (sockaddr *) &servaddr, sizeof(servaddr));
    // fprintf(stdout, "Hello I'm in!\n");
    char msg[256];
    strcpy(msg, "S");
    send(sockfd, msg, sizeof(msg), 0);
    recv(sockfd, msg, 256, 0);
    memmove(msg, msg + 4, strlen(msg));
    for(int i = 0;i < 9;i++){
        for(int j = 0;j < 9;j++){
            // fprintf(stdout,"%d th is %c\n", i * 9 + j, msg[i * 9 + j]);
            if(msg[i * 9 + j] == '.'){
                continue;
            }
            else{
                grid[i][j] = msg[i * 9 + j] - '0';
            }
        }
        
    }
    // fprintf(stdout, "%s", msg);

    

    // print_grid(grid);

	if (true == solve_soduko(grid))
	{
		print_grid(grid);
        char cmd[128];
        for(int i = 0;i < 9;i++){
            for(int j = 0;j < 9;j++){                
                sprintf(cmd,"V %d %d %d", i, j, grid[i][j]);
                send(sockfd, cmd, sizeof(cmd), MSG_WAITALL);
                // sleep(1);
            }            
        }
        strcpy(cmd, "C");
        send(sockfd, cmd, sizeof(msg), MSG_WAITALL);
	}
	else
	{
        fprintf(stdout, "No solution exists for the given Soduko\n\n");
	}

    exit(0);
}