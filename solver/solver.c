#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

void print(int arr[9][9]) //for debug
{
     for (int i = 0; i < 9; i++)
      {
         for (int j = 0; j < 9; j++)
            printf("%d ",arr[i][j]);
         printf("\n");
       }
}

int validSquare(int **board, int i, int j) //auxiliary function for validBoard
{
    int digits[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (int x = j; x < j + 3; x++)
    {
        if (digits[board[i][x]] == 0)
        {
            digits[board[i][x]] = board[i][x];
        }
        else
        {
            return 1;
        }
    }
    for (int x = j; x < j + 3; x++)
    {
        if (digits[board[i + 1][x]] == 0)
        {
            digits[board[i + 1][x]] = board[i + 1][x];
        }
        else
        {
            return 1;
        }
    }
    for (int x = j; x < j + 3; x++)
    {
        if (digits[board[i + 2][x]] == 0)
        {
            digits[board[i + 2][x]] = board[i + 2][x];
        }
        else
        {
            return 1;
        }
    }
    return 0;
}

int validBoard(int **board) //returns 0  if the board is valid and a positive number if not
{
    int res = 0;
    int digits[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (int j = 0; j <  9; j++) //checks columns
    {
        for (int i = 0; i < 9; i++)
        {
            if (digits[board[i][j]] != 0)
            {
                res++;
                return res;
            }
            digits[board[i][j]] = board[i][j];
        }
        for (int i = 0; i < 10; i++)
        {
            digits[i] = 0;
        }
    }

    for (int j = 0; j <  9; j++) //checks rows
    {
        for (int i = 0; i < 9; i++)
        {
            if (digits[board[j][i]] != 0)
            {
                res++;
                return res;
            }
            digits[board[j][i]] = board[j][i];
        }
        for (int i = 0; i < 10; i++)
        {
            digits[i] = 0;
        }
    }

    res += validSquare(board, 0, 0);
    res += validSquare(board, 0, 3);
    res += validSquare(board, 0, 6);
    res += validSquare(board, 3, 0);
    res += validSquare(board, 6, 0);
    res += validSquare(board, 3, 3);
    res += validSquare(board, 3, 6);
    res += validSquare(board, 6, 3);
    res += validSquare(board, 6, 6);
    return res;
}

int solve(int n, int p, int **board) // return 0 if board was solved and 1 if not
{
    if (n == 8 && p == 9)
    {
        return 0;
    }
    if (p == 9)
    {
        n++;
        p = 0;
    }
    if (board[n][p] != 0)
    {
        return solve(n, p + 1, board);
    }

    for (int i = 1; i < 10; i++)
    {
        int tmp = board[n][p];
        board[n][p] = i;
        int bool = validBoard(board);
        board[n][p] = tmp;
        if (bool == 0)
        {   
            board[n][p] = i;
            if (solve(n, p+1, board) == 0)
            {
                return 0;
            }
        }
        board[n][p] = 0;
    }
    return 1;
}

/*
int main(int argc, char** argv)
{
    if (argc != 2)
    {
        errx(1, "Usage : ./solver filename");
    }
    else
    {
        char chr;
        FILE* f = fopen(argv[1], "r");
        int board[9][9];
        int i = 0;
        int j = 0;
        do
        {
            chr = fgetc(f);
            if (chr != ' ' && chr != '\n' && chr != EOF)
            {
                board[i][j] = chr == '.' ? 0 : chr - 48;   
                if (j < 8)
                {
                    j++;
                }
                else
                {
                    i++;
                    j = 0;
                }
            }
        } while(chr != EOF);
        fclose(f);
        solve(0, 0, board);
        char *output_name = strcat(argv[1], ".result");
        FILE *output = fopen(output_name, "w");
        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                if (j == 3 || j == 6)
                {
                    fprintf(output, " ");
                }
                fprintf(output, "%i", board[i][j]);
            }
            fprintf(output, "\n");
            if (i == 2 || i == 5)
            {
                fprintf(output, "\n");
            }
        }
        fclose(output);
    }
    return 0;
}
*/