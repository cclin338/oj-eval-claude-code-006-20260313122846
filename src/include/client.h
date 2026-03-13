#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

// Custom variables for client
char client_map[35][35];    // Current known state of map
bool is_mine[35][35];        // Known mines
bool is_safe[35][35];        // Known safe blocks
int remaining_mines;         // Mines not yet marked
int total_unknown;           // Unknown blocks remaining

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  // Initialize all global variables
  remaining_mines = total_mines;
  total_unknown = rows * columns;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      client_map[i][j] = '?';
      is_mine[i][j] = false;
      is_safe[i][j] = false;
    }
  }

  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  total_unknown = 0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      char c;
      std::cin >> c;
      client_map[i][j] = c;
      if (c == '?') {
        total_unknown++;
      } else if (c == '@') {
        is_mine[i][j] = true;
      }
    }
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
  int dc[] = {-1, 0, 1, -1, 1, -1, 0, 1};

  // Strategy 1: Use auto-explore when all mines around a cell are marked
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] >= '0' && client_map[i][j] <= '8') {
        int num = client_map[i][j] - '0';
        int unknown_count = 0;
        int mine_count = 0;

        for (int k = 0; k < 8; k++) {
          int ni = i + dr[k];
          int nj = j + dc[k];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (client_map[ni][nj] == '?') unknown_count++;
            else if (client_map[ni][nj] == '@') mine_count++;
          }
        }

        // All mines found, auto-explore to visit remaining safe cells
        if (mine_count == num && unknown_count > 0) {
          Execute(i, j, 2);
          return;
        }
      }
    }
  }

  // Strategy 2: Mark obvious mines
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] >= '0' && client_map[i][j] <= '8') {
        int num = client_map[i][j] - '0';
        int unknown_count = 0;
        int mine_count = 0;

        for (int k = 0; k < 8; k++) {
          int ni = i + dr[k];
          int nj = j + dc[k];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (client_map[ni][nj] == '?') unknown_count++;
            else if (client_map[ni][nj] == '@') mine_count++;
          }
        }

        // All unknowns must be mines
        if (mine_count + unknown_count == num && unknown_count > 0) {
          for (int k = 0; k < 8; k++) {
            int ni = i + dr[k];
            int nj = j + dc[k];
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && client_map[ni][nj] == '?') {
              Execute(ni, nj, 1);
              return;
            }
          }
        }
      }
    }
  }

  // Strategy 3: Pattern-based deduction (subset relations)
  // For each pair of numbered cells, check if one's unknowns are a subset of another's
  for (int i1 = 0; i1 < rows; i1++) {
    for (int j1 = 0; j1 < columns; j1++) {
      if (client_map[i1][j1] < '0' || client_map[i1][j1] > '8') continue;

      int num1 = client_map[i1][j1] - '0';
      int unknown1[8], uk1_cnt = 0, mine1_cnt = 0;

      for (int k = 0; k < 8; k++) {
        int ni = i1 + dr[k], nj = j1 + dc[k];
        if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
          if (client_map[ni][nj] == '?') unknown1[uk1_cnt++] = ni * 100 + nj;
          else if (client_map[ni][nj] == '@') mine1_cnt++;
        }
      }

      if (uk1_cnt == 0) continue;
      int remain1 = num1 - mine1_cnt;

      for (int i2 = 0; i2 < rows; i2++) {
        for (int j2 = 0; j2 < columns; j2++) {
          if (i1 == i2 && j1 == j2) continue;
          if (client_map[i2][j2] < '0' || client_map[i2][j2] > '8') continue;

          int num2 = client_map[i2][j2] - '0';
          int unknown2[8], uk2_cnt = 0, mine2_cnt = 0;

          for (int k = 0; k < 8; k++) {
            int ni = i2 + dr[k], nj = j2 + dc[k];
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
              if (client_map[ni][nj] == '?') unknown2[uk2_cnt++] = ni * 100 + nj;
              else if (client_map[ni][nj] == '@') mine2_cnt++;
            }
          }

          if (uk2_cnt == 0) continue;
          int remain2 = num2 - mine2_cnt;

          // Check if unknown1 is subset of unknown2
          bool is_subset = true;
          for (int a = 0; a < uk1_cnt; a++) {
            bool found = false;
            for (int b = 0; b < uk2_cnt; b++) {
              if (unknown1[a] == unknown2[b]) {
                found = true;
                break;
              }
            }
            if (!found) {
              is_subset = false;
              break;
            }
          }

          if (is_subset && uk1_cnt < uk2_cnt) {
            int diff_cnt = uk2_cnt - uk1_cnt;
            int diff_mines = remain2 - remain1;

            // The difference cells are all safe
            if (diff_mines == 0) {
              for (int b = 0; b < uk2_cnt; b++) {
                bool in_uk1 = false;
                for (int a = 0; a < uk1_cnt; a++) {
                  if (unknown2[b] == unknown1[a]) {
                    in_uk1 = true;
                    break;
                  }
                }
                if (!in_uk1) {
                  int r = unknown2[b] / 100;
                  int c = unknown2[b] % 100;
                  Execute(r, c, 0);
                  return;
                }
              }
            }

            // The difference cells are all mines
            if (diff_mines == diff_cnt) {
              for (int b = 0; b < uk2_cnt; b++) {
                bool in_uk1 = false;
                for (int a = 0; a < uk1_cnt; a++) {
                  if (unknown2[b] == unknown1[a]) {
                    in_uk1 = true;
                    break;
                  }
                }
                if (!in_uk1) {
                  int r = unknown2[b] / 100;
                  int c = unknown2[b] % 100;
                  Execute(r, c, 1);
                  return;
                }
              }
            }
          }
        }
      }
    }
  }

  // Strategy 4: Pick lowest risk cell
  int best_r = -1, best_c = -1;
  double min_risk = 2.0;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] == '?') {
        int adjacent_nums = 0;
        double risk_sum = 0;

        for (int k = 0; k < 8; k++) {
          int ni = i + dr[k];
          int nj = j + dc[k];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns &&
              client_map[ni][nj] >= '0' && client_map[ni][nj] <= '8') {
            int num = client_map[ni][nj] - '0';
            int unknown = 0, mines = 0;

            for (int m = 0; m < 8; m++) {
              int nni = ni + dr[m];
              int nnj = nj + dc[m];
              if (nni >= 0 && nni < rows && nnj >= 0 && nnj < columns) {
                if (client_map[nni][nnj] == '?') unknown++;
                else if (client_map[nni][nnj] == '@') mines++;
              }
            }

            if (unknown > 0) {
              adjacent_nums++;
              risk_sum += (double)(num - mines) / unknown;
            }
          }
        }

        double risk = (adjacent_nums > 0) ? risk_sum / adjacent_nums : 0.5;

        // Prefer cells with more information (adjacent numbered cells)
        double score = risk - adjacent_nums * 0.01;

        if (score < min_risk) {
          min_risk = score;
          best_r = i;
          best_c = j;
        }
      }
    }
  }

  if (best_r != -1 && best_c != -1) {
    Execute(best_r, best_c, 0);
    return;
  }

  // Fallback: visit any unknown
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] == '?') {
        Execute(i, j, 0);
        return;
      }
    }
  }
}

#endif