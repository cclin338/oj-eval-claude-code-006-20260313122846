// Improved Decide function with advanced constraint solving

void Decide() {
  int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
  int dc[] = {-1, 0, 1, -1, 1, -1, 0, 1};

  // Reset probability matrix
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      prob_mine[i][j] = -1.0;  // Unknown
    }
  }

  // Phase 1: Auto-explore when all mines marked
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

        if (mine_count == num && unknown_count > 0) {
          Execute(i, j, 2);
          return;
        }
      }
    }
  }

  // Phase 2: Mark obvious mines
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

  // Phase 3: Advanced subset/superset constraint solving
  for (int iter = 0; iter < 3; iter++) {
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

              // Difference cells are all safe
              if (diff_mines == 0 && diff_cnt > 0) {
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

              // Difference cells are all mines
              if (diff_mines == diff_cnt && diff_cnt > 0) {
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
  }

  // Phase 4: Calculate mine probabilities for each unknown cell
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] == '?') {
        int constraint_count = 0;
        double prob_sum = 0.0;

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
              constraint_count++;
              prob_sum += (double)(num - mines) / unknown;
            }
          }
        }

        if (constraint_count > 0) {
          prob_mine[i][j] = prob_sum / constraint_count;
        } else {
          // No adjacent constraints, use global probability
          int marked_count = 0;
          for (int ii = 0; ii < rows; ii++) {
            for (int jj = 0; jj < columns; jj++) {
              if (client_map[ii][jj] == '@') marked_count++;
            }
          }
          prob_mine[i][j] = (double)(total_mines - marked_count) / total_unknown;
        }
      }
    }
  }

  // Phase 5: Pick cell with lowest mine probability
  double min_prob = 2.0;
  int best_r = -1, best_c = -1;
  int max_info = -1;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] == '?' && prob_mine[i][j] >= 0) {
        // Count adjacent numbered cells for information value
        int info_count = 0;
        for (int k = 0; k < 8; k++) {
          int ni = i + dr[k];
          int nj = j + dc[k];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns &&
              client_map[ni][nj] >= '0' && client_map[ni][nj] <= '8') {
            info_count++;
          }
        }

        // Prefer cells with lowest probability
        // Break ties by preferring cells with more information
        if (prob_mine[i][j] < min_prob - 0.001 ||
            (prob_mine[i][j] < min_prob + 0.001 && info_count > max_info)) {
          min_prob = prob_mine[i][j];
          max_info = info_count;
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
