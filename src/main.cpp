#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <random>

class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
    std::mutex board_mutex; // Mutex para controle de acesso ao tabuleiro
    std::condition_variable turn_cv; // Variável de condição para alternância de turnos
    char current_player; // Jogador atual ('X' ou 'O')
    bool game_over; // Estado do jogo
    char winner; // Vencedor do jogo

public:
    TicTacToe() : current_player('X'), game_over(false), winner(' ') {
        for (auto& row : board) row.fill(' ');
    }

    void display_board() {
        for (size_t i = 0; i < board.size(); ++i) {
            for (size_t j = 0; j < board[i].size(); ++j) {
                std::cout << board[i][j];
                if (j < board[i].size() - 1) std::cout << '|';
            }
            std::cout << '\n';
            if (i < board.size() - 1) std::cout << "-----\n";
        }
        std::cout << "\n";
    }

    bool make_move(char player, int row, int col) {
        std::unique_lock<std::mutex> lock(board_mutex);
        if (game_over) return false;
        turn_cv.wait(lock, [&] { return player == current_player || game_over; });
        if (game_over) return false;

        if (board[row][col] == ' ') {
            board[row][col] = player;
            display_board();

            if (check_win(player)) {
                game_over = true;
                winner = player;
            } else if (check_draw()) {
                game_over = true;
                winner = 'D';
            }

            current_player = (player == 'X') ? 'O' : 'X'; // Alterna o turno
            lock.unlock();
            turn_cv.notify_all();
            return true;
            } 
            else {
            std::cout << "Posicao ja ocupada! Tente outra.\n";
            return false;
        }
    }

    bool check_win(char player) {
        // Check linhas
        for (int i = 0; i < 3; i++) {
            if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] == player) {
                return true;
            }
        }

        // Check colunas
        for (int i = 0; i < 3; i++) {
            if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] == player) {
                return true;
            }
        }

        // Check diagonal principal
        if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] == player) {
            return true;
        }

        // Check diagonal secundaria
        if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] == player) {
            return true;
        }

        return false;
    }

    bool check_draw() {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (board[i][j] == ' ') { // Se há um espaço vazio, não é empate
                    return false;
                }
            }
        }
        return true; // Nenhum espaço vazio, é empate
    }

    bool is_game_over() {
        return game_over;
    }

    char get_winner() {
        return winner;
    }
};


// Classe Player
class Player {
private:
    TicTacToe& game; // Referência para o jogo
    char symbol; // Símbolo do jogador ('X' ou 'O')
    std::string strategy; // Estratégia do jogador
    std::random_device rd;
    std::mt19937 gen;

public:
    Player(TicTacToe& g, char s, std::string strat) 
        : game(g), symbol(s), strategy(strat), gen(rd()) {}

    void play() {
        // Executar jogadas de acordo com a estratégia escolhida
        if (strategy == "sequential") {
            play_sequential();
        } else if (strategy == "random") {
            play_random();
        }
    }

private:
    void play_sequential() {
        // Implementar a estratégia sequencial de jogadas
        for (int i = 0; i < 3 && !game.is_game_over(); ++i) {
            for (int j = 0; j < 3 && !game.is_game_over(); ++j) {
                if (game.make_move(symbol, i, j)) {
                    //game.display_board();
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            }
        }
    }

    void play_random() {
        std::uniform_int_distribution<> dis(0, 2);
        while (!game.is_game_over()) {
            int row = dis(gen);
            int col = dis(gen);
            if (game.make_move(symbol, row, col)) {
                //game.display_board();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
    }
};

// Função principal
int main() {
    // Inicializar o jogo e os jogadores
    TicTacToe game;

    Player playerX(game, 'X', "sequential");
    Player playerO(game, 'O', "random");
    // Criar as threads para os jogadores
    std::thread t1(&Player::play, &playerX);
    std::thread t2(&Player::play, &playerO);

    t1.join();
    t2.join();
    // Aguardar o término das threads

    // Exibir o resultado final do jogo
    std::cout << "Resultado final:\n";
    game.display_board();
    char winner = game.get_winner();
    if (winner == 'D') {
        std::cout << "Empate!\n";
    } else {
        std::cout << "Vencedor: " << winner << '\n';
    }

    return 0;
}
