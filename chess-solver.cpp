#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_MOVES_LOG_SIZE  1000000
#define MAX_MOVES           6
#define BOARD_SIZE          8

typedef enum
{
  UNKNOWN_PIECE=' ', KING='k', QUEEN='q', ROOK='r', BISHOP='b', KNIGHT='n', PAWN='p'
} PieceType_e;

typedef enum
{
  UNKNOWN_COLOR=0, BLACK=1, WHITE=2
} Color_e;

typedef enum
{
  SAFE=0, DANGER=1
} Status_e;


typedef struct board_s
{
  PieceType_e piece_type;
  Color_e     piece_color;
  Status_e    black_status;
  Status_e    white_status;
  int         can_go_to[30];
  int         can_go_to_count;
} Board_t[BOARD_SIZE][BOARD_SIZE];

typedef struct
{
  int i;
  int from;
  int to;
} move_t;

Board_t board;
Board_t move_board[MAX_MOVES*2];
int move_count = 1;
move_t moves_log[MAX_MOVES_LOG_SIZE];
long moves_log_count = 0;
char pos[3];

int find_fatal_white_move(int move_num);
int find_safe_black_move(int move_num);

int is_valid(char c)
{
  c = tolower(c);
  if (c==UNKNOWN_PIECE || c==KING || c==QUEEN || c==ROOK || c==BISHOP || c==KNIGHT || c==PAWN)
    return 1;

  return 0;
}

void read_board(char *filename)
{
  FILE *fp = fopen(filename, "r");
  if (!fp)
  {
    printf("File \"%s\" ", filename);
    fflush(stdout);
    perror("could not be opened");
    exit(1);
  }

  for (int x=0; x<BOARD_SIZE; x++)
  {
    char s[128];
    fgets(s, 128, fp);
    int i=0;
    for (int y=0; y<BOARD_SIZE; y++)
    {
      board[x][y].piece_type = UNKNOWN_PIECE;
      board[x][y].piece_color = UNKNOWN_COLOR;
      board[x][y].black_status = SAFE;
      board[x][y].white_status = SAFE;
      while (!is_valid(s[i]))
        i++;
      if (s[i] != UNKNOWN_PIECE)
      {
        board[x][y].piece_type = (PieceType_e)tolower(s[i]);
        if (islower(s[i]))
          board[x][y].piece_color=BLACK;
        else
          board[x][y].piece_color=WHITE;
      }
      i++;
    }
  }

  fclose (fp);
}

char *str_piece_type(int x)
{
  switch (board[x/BOARD_SIZE][x%BOARD_SIZE].piece_type)
  {
    case KING:   return "king   ";
    case ROOK:   return "rook   ";
    case BISHOP: return "bishop ";
    case QUEEN:  return "queen  ";
    case KNIGHT: return "knight ";
    case PAWN:   return "pawn   ";
  }

  return "";
}

char *str_pos(int x)
{
  sprintf(pos, "%c%d", (x%BOARD_SIZE)+'a', BOARD_SIZE-(x/BOARD_SIZE));
  return pos;
}

void print_board()
{
  printf("------------------------------------\n");
  for (int x=0; x<BOARD_SIZE; x++)
  {
    for (int y=0; y<BOARD_SIZE; y++)
    {
      printf("|");
      if (board[x][y].piece_color == UNKNOWN_COLOR)
      {
        printf(" %d%d ", board[x][y].black_status, board[x][y].white_status);
      }
      else if (board[x][y].piece_color == BLACK)
      {
        printf("%c%d%d ", board[x][y].piece_type, board[x][y].black_status, board[x][y].white_status);
      }
      else if (board[x][y].piece_color == WHITE)
      {
        printf("%c%d%d ", toupper(board[x][y].piece_type), board[x][y].black_status, board[x][y].white_status);
      }
    }
    printf("|  %d\n", BOARD_SIZE-x);
  }
  printf("--a----b----c----d----e----f----g----h------\n");
  for (int x=0; x<BOARD_SIZE; x++)
  {
    for (int y=0; y<BOARD_SIZE; y++)
    {
      if (board[x][y].piece_color != UNKNOWN_COLOR)
      {
        printf("\n%s @%s -> ", str_piece_type(x*BOARD_SIZE+y), str_pos(x*BOARD_SIZE+y));
        for (int i=0; i<board[x][y].can_go_to_count; i++)
          printf("%s, ", str_pos(board[x][y].can_go_to[i]));
      }
    }
  }
  printf("\n------------------------------------\n");
}

void get_x_y(int i, int *x, int *y)
{
  *x = (i/BOARD_SIZE);
  *y = (i%BOARD_SIZE);
}

void add_move(int i, int from, int to)
{
  if (moves_log_count == MAX_MOVES_LOG_SIZE)
  {
    printf("Max moves log size reached\n\n");
    exit(0);
  }

  moves_log[moves_log_count].i = i;
  moves_log[moves_log_count].from = from;
  moves_log[moves_log_count].to = to;

  moves_log_count++;
}

void print_moves()
{
  for (int i=moves_log_count-1; i>=0; i--)
  {
    printf("Move %d: %s -> ", moves_log[i].i, str_pos(moves_log[i].from));
    printf("%s\n", str_pos(moves_log[i].to));
  }
}

void can_go(int x, int y, int to_x, int to_y)
{
  if (to_x>=0 && to_x<BOARD_SIZE && to_y>=0 && to_y<BOARD_SIZE)
  {
    if (board[x][y].piece_color != board[to_x][to_y].piece_color)
    {
      int count = board[x][y].can_go_to_count;
      if (board[x][y].piece_type == PAWN)
      {
        if ((y==to_y && board[to_x][to_y].piece_type == UNKNOWN_PIECE) ||
            (y!=to_y && board[to_x][to_y].piece_color != UNKNOWN_COLOR))
        {
          board[x][y].can_go_to[count] = to_x*BOARD_SIZE+to_y;
          board[x][y].can_go_to_count++;
        }
      }
      else
      {
        board[x][y].can_go_to[count] = to_x*BOARD_SIZE+to_y;
        board[x][y].can_go_to_count++;
      }
    }
    if (board[x][y].piece_color == WHITE)
    {
      if (board[x][y].piece_type == PAWN && y==to_y);
      else
        board[to_x][to_y].black_status = DANGER;
    }
    else if (board[x][y].piece_color == BLACK)
    {
      if (board[x][y].piece_type == PAWN && y==to_y);
      else
        board[to_x][to_y].white_status = DANGER;
    }
  }
}

void can_go_rook(int x, int y)
{
  for (int i=x+1; i<BOARD_SIZE; i++)
  {
    if (board[i][y].piece_color == UNKNOWN_COLOR)
      can_go(x, y, i, y);
    else if (board[i][y].piece_color != board[x][y].piece_color)
    {
      can_go(x, y, i, y); break;
    }
    else
      break;
  }
  for (int i=x-1; i>=0; i--)
  {
    if (board[i][y].piece_color == UNKNOWN_COLOR)
      can_go(x, y, i, y);
    else if (board[i][y].piece_color != board[x][y].piece_color)
    {
      can_go(x, y, i, y); break;
    }
    else
      break;
  }
  for (int j=y+1; j<BOARD_SIZE; j++)
  {
    if (board[x][j].piece_color == UNKNOWN_COLOR)
      can_go(x, y, x, j);
    else if (board[x][j].piece_color != board[x][y].piece_color)
    {
      can_go(x, y, x, j); break;
    }
    else
      break;
  }
  for (int j=y-1; j>=0; j--)
  {
    if (board[x][j].piece_color == UNKNOWN_COLOR)
      can_go(x, y, x, j);
    else if (board[x][j].piece_color != board[x][y].piece_color)
    {
      can_go(x, y, x, j); break;
    }
    else
      break;
  }
}

void can_go_bishop(int x, int y)
{
  for (int i=x+1,j=y+1; i<BOARD_SIZE && j<BOARD_SIZE; i++,j++)
  {
    if (board[i][j].piece_color == UNKNOWN_COLOR)
      can_go(x, y, i, j);
    else if (board[i][j].piece_color != board[x][y].piece_color)
    {
      can_go(x, y, i, j); break;
    }
    else
      break;
  }
  for (int i=x+1,j=y-1; i<BOARD_SIZE && j>=0; i++,j--)
  {
    if (board[i][j].piece_color == UNKNOWN_COLOR)
      can_go(x, y, i, j);
    else if (board[i][j].piece_color != board[x][y].piece_color)
    {
      can_go(x, y, i, j); break;
    }
    else
      break;
  }
  for (int i=x-1,j=y+1; i>=0 && j<BOARD_SIZE; i--,j++)
  {
    if (board[i][j].piece_color == UNKNOWN_COLOR)
      can_go(x, y, i, j);
    else if (board[i][j].piece_color != board[x][y].piece_color)
    {
      can_go(x, y, i, j); break;
    }
    else
      break;
  }
  for (int i=x-1,j=y-1; i>=0 && j>=0; i--,j--)
  {
    if (board[i][j].piece_color == UNKNOWN_COLOR)
      can_go(x, y, i, j);
    else if (board[i][j].piece_color != board[x][y].piece_color)
    {
      can_go(x, y, i, j); break;
    }
    else
      break;
  }
}

void scan_board()
{
  for (int x=0; x<BOARD_SIZE; x++)
  {
    for (int y=0; y<BOARD_SIZE; y++)
    {
      board[x][y].black_status = SAFE;
      board[x][y].white_status = SAFE;
      board[x][y].can_go_to_count = 0;
    }
  }

  for (int x=0; x<BOARD_SIZE; x++)
  {
    for (int y=0; y<BOARD_SIZE; y++)
    {
        switch (board[x][y].piece_type)
        {
          case KING:
          {
            can_go(x, y, x-1, y-1);
            can_go(x, y, x-1, y);
            can_go(x, y, x-1, y+1);
            can_go(x, y, x, y-1);
            can_go(x, y, x, y+1);
            can_go(x, y, x+1, y-1);
            can_go(x, y, x+1, y);
            can_go(x, y, x+1, y+1);
            break;
          }
          case ROOK:
          {
            can_go_rook(x, y);
            break;
          }
          case BISHOP:
          {
            can_go_bishop(x, y);
            break;
          }
          case QUEEN:
          {
            can_go_rook(x, y);
            can_go_bishop(x, y);
            break;
          }
          case KNIGHT:
          {
            can_go(x, y, x-2, y-1);
            can_go(x, y, x-2, y+1);
            can_go(x, y, x+2, y-1);
            can_go(x, y, x+2, y+1);
            can_go(x, y, x-1, y-2);
            can_go(x, y, x+1, y-2);
            can_go(x, y, x-1, y+2);
            can_go(x, y, x+1, y+2);
            break;
          }
          case PAWN:
          {
            if (board[x][y].piece_color == WHITE)
            {
              can_go(x, y, x-1, y-1);
              can_go(x, y, x-1, y);
              can_go(x, y, x-1, y+1);
            }
            else
            {
              can_go(x, y, x+1, y-1);
              can_go(x, y, x+1, y);
              can_go(x, y, x+1, y+1);
            }
            break;
          }
        }
    }
  }
}

void move(int from, int to)
{
  int from_x, from_y, to_x, to_y;
  get_x_y(from, &from_x, &from_y);
  get_x_y(to, &to_x, &to_y);
  board[to_x][to_y].piece_type = board[from_x][from_y].piece_type;
  board[to_x][to_y].piece_color = board[from_x][from_y].piece_color;
  board[from_x][from_y].piece_type = UNKNOWN_PIECE;
  board[from_x][from_y].piece_color = UNKNOWN_COLOR;
  scan_board();
}

void get_king_pos(Color_e color, int *x, int *y)
{
  for (int i=0; i<BOARD_SIZE; i++)
  {
    for (int j=0; j<BOARD_SIZE; j++)
    {
      if (board[i][j].piece_type == KING && board[i][j].piece_color == color)
      {
        *x = i; *y = j;
        return;
      }
    }
  }
  return;
}

int is_black_safe()
{
  int x,y=0;
  get_king_pos(BLACK, &x, &y);
  if (board[x][y].black_status == DANGER)
    return 0;

  return 1;
}

int is_white_safe()
{
  int x,y=0;
  get_king_pos(WHITE, &x, &y);
  if (board[x][y].white_status == DANGER)
    return 0;

  return 1;
}

int find_safe_black_move(int move_num)
{
  int c = moves_log_count;
  for (int from=0; from<BOARD_SIZE*BOARD_SIZE; from++)
  {
    int x, y=0;
    get_x_y(from, &x, &y);
    if (board[x][y].piece_color == BLACK)
    {
      for (int j=0; j<board[x][y].can_go_to_count; j++)
      {
        int to = board[x][y].can_go_to[j];
        move(from, to);
        if (is_black_safe())
        {
          if (move_num == 2*move_count)
            return 1;

          memcpy(&move_board[move_num], &board, sizeof(board));
          if (find_fatal_white_move(move_num+1))
          {
            add_move(move_num, from, to);
          }
          else
          {
            moves_log_count = c;
            return 1;
          }
        }
        memcpy(&board, &move_board[move_num-1], sizeof(board));
      }
    }
  }
  return 0;
}

int find_fatal_white_move(int move_num)
{
  for (int from=0; from<BOARD_SIZE*BOARD_SIZE; from++)
  {
    int x, y=0;
    get_x_y(from, &x, &y);
    if (board[x][y].piece_color == WHITE)
    {
      for (int i=0; i<board[x][y].can_go_to_count; i++)
      {
        int to = board[x][y].can_go_to[i];
        move(from, to);
        if (is_white_safe())
        {
          memcpy(&move_board[move_num], &board, sizeof(board));
          if (find_safe_black_move(move_num+1)) ;
          else
          {
            add_move(move_num, from, to);
            return 1;
          }
        }
        memcpy(&board, &move_board[move_num-1], sizeof(board));
      }
    }
  }
  return 0;
}

int main(int argc, char **argv)
{
  if (argc <= 1)
  {
    printf("\nUsage: %s <file>\n\n", argv[0]);
    printf("Input file is in this format:\n\n");
    printf("| |N| | |R| | | |\n");
    printf("| | | | | |n| | |\n");
    printf("| |K| |k| | | | |\n");
    printf("| | | |p|p|p| | |\n");
    printf("| | |p| | | | | |\n");
    printf("| | | | | | | | |\n");
    printf("| | | | | |R| | |\n");
    printf("| | | | | | | |Q|\n\n");
    printf("K=King, Q=Queen, R=Rook, B=Bishop, N=Knight, P=Pawn (WHITE)\n");
    printf("k=King, q=Queen, r=Rook, b=Bishop, n=Knight, p=Pawn (BLACK)\n\n");

    exit(1);
  }

  read_board(argv[1]);
  scan_board();
//  print_board();
  fflush(stdout);

  memcpy(&move_board[0], &board, sizeof(board));
  memset(&moves_log, 0, sizeof(moves_log));

  while (1)
  {
    if (find_fatal_white_move(1))
    {
      printf("Found moves to mate in %d moves...\n", move_count);
      print_moves();
      break;
    }
    else
    {
      if (move_count < MAX_MOVES)
      {
        printf("Could not find moves to mate in %d moves...Looking for moves to mate in %d moves...\n", move_count, move_count+1);
        fflush(stdout);
        move_count++;
      }
      else
      {
        printf("Could not find moves to mate in %d moves...Exiting...\n", move_count);
        fflush(stdout);
        break;
      }
    }
  }

  return 0;
}