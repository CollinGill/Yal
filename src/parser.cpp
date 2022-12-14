#include "./include/parser.h"

Parser::Parser() 
{
    ast = AST();
    token_list = std::vector<token::Token>();
    special_chars = {'(', ')', '+', '-', '*', '/', '%'};
}

void Parser::parse(std::string& file)
{
    tokenize(file);
    ast.generate_tree(token_list);
}

void Parser::evaluate()
{
    eval_ast(ast.get_root());
    std::cout << ast.get_root()->tok.get_val() << std::endl;
}

std::string Parser::read_file(std::string& file_name)
{
    if (!(file_name.ends_with(".yal"))) {
        std::cout << "ERROR: file must end with '.yal'\n\n";
        assert(false);
    }

    std::ifstream file {file_name};
    std::string file_txt { "" };
    std::string line;

    if (file.is_open()) {
        while (std::getline(file, line))
            file_txt.append(line);

        return file_txt;

    } else {
        std::cout << "ERROR: Unable to open file `" << file_name << "`\n\n";
        assert(false);
    }
}

void Parser::eval_ast(node::Node *rt)
{
    if (rt == nullptr)
        return;

    if (rt->tok.get_type() == token::LIST) {
        for (auto child : rt->children)
            eval_ast(child);
        
        rt->tok = eval_list(rt->children);

    } else {
        return;
    }
}

token::Token Parser::eval_list(std::vector<node::Node*> &children)
{
    token::Token ret_tok { token::Token(token::NONE, "") };
    if (children.empty())
        return ret_tok;

    token::Token func_tok { children.at(0)->tok };
    children = std::vector<node::Node*>(children.begin() + 1, children.end());

    // The function is a math operator
    if (token::operators.find(func_tok.get_type()) != token::operators.end()) {
        float count = 0;
        switch (func_tok.get_type())
        {
            case token::PLUS:
                for (auto child : children) {
                    if (child->tok.get_val()[0] == '-')
                        count += (-1 * std::stof(child->tok.get_val().substr(1)));
                    else
                        count += std::stof(child->tok.get_val());
                }
                break;
            case token::MINUS:
                count = std::stof(children.at(0)->tok.get_val());
                for (std::size_t i = 1; i < children.size(); i++) {
                    if (children.at(0)->tok.get_val()[0] == '-')
                        count -= -1 * std::stof(children.at(i)->tok.get_val().substr(1));
                    else
                        count -= std::stof(children.at(i)->tok.get_val());
                }
                break;
            case token::MULTIPLY:
                count = 1;
                for (auto child : children) {
                    if (child->tok.get_val()[0] == '-')
                        count *= -1 * std::stof(child->tok.get_val().substr(1));
                    else
                        count *= std::stof(child->tok.get_val());
                }
                break;
            case token::DIVIDE:
                count = std::stof(children.at(0)->tok.get_val());
                for (std::size_t i = 1; i < children.size(); i++) {
                    if (children.at(0)->tok.get_val()[0] == '-')
                        count /= -1 * std::stof(children.at(i)->tok.get_val().substr(1));
                    else
                        count /= std::stof(children.at(i)->tok.get_val());
                }
                break;
            default:
                break;
        }
        bool is_flt { count - (int)count > 0 };
        ret_tok.set_type((is_flt) ? token::FLOAT : token::INTEGER);
        std::string new_val = (is_flt) ? std::to_string(count) : std::to_string((int)count);
        ret_tok.set_val(new_val);
    }
    return ret_tok;
}

void Parser::tokenize(std::string& file)
{
    int paren_count { 0 };
    int quote_count { 0 };
    bool in_quote { false };
    
    for (std::size_t i = 0; i < file.size(); i++) {
        char first { file[i] };
        switch (first) {
            case '(':
            {
                paren_count++;
                token::Token tmp { token::Token(token::LPAREN, std::string(1, first)) };
                token_list.push_back(tmp);
                continue;
            }

            case ')':
            {
                paren_count--;

                if (paren_count < 0) {
                    std::cout << "ERROR: Unbalanced parenthesis\n";
                    assert(false);
                }
                token::Token tmp { token::Token(token::RPAREN, std::string(1, first)) };
                token_list.push_back(tmp);

                continue;
            }

            case ' ':
            case '\n':
            case '\t':
            {
                continue;
            }
            
            case '+':
            {
                token::Token tmp { token::Token(token::PLUS, std::string(1, first)) };
                token_list.push_back(tmp);
                continue;
            }

            case '-':
            {
                if (i < file.size() - 1) {
                    if (isdigit(file[i+1]))
                        break;
                }

                token::Token tmp { token::Token(token::MINUS, std::string(1, first)) };
                token_list.push_back(tmp);
                continue;
            }                

            case '*':
            {
                token::Token tmp { token::Token(token::MULTIPLY, std::string(1, first)) };
                token_list.push_back(tmp);
                continue;
            }

            case '/':
            {
                token::Token tmp { token::Token(token::DIVIDE, std::string(1, first)) };
                token_list.push_back(tmp);
                continue;
            }

            case '%':
            {
                token::Token tmp { token::Token(token::MODULO, std::string(1, first)) };
                token_list.push_back(tmp);
                continue;
            }

            case '"':
            {
                in_quote = !(in_quote);
                quote_count++;
                continue;
            }

            default:
                break;
        }

        // Iterate till end of word or file
        for (std::size_t j = i; i < file.size() && j < file.size(); j++) {
            char last { file[j] };

            if (last == '"') {
                in_quote = !(in_quote);
                quote_count--;
                if (quote_count < 0) {
                    std::cout << "ERROR: Unbalanced quotes\n";
                    assert(false);
                }

                token::Token tmp { token::Token(token::STRING_LITERAL, file.substr(i, (j-i))) };
                token_list.push_back(tmp);

                // i = j so that on the next loop it skips the second quote symbol
                i = j;
                break;

            } else if ((last == ' ' || last == '\n' || last == '\t' || special_chars.find(last) != special_chars.end()) && !(in_quote) && last != '-') {
                std::string word_name = file.substr(i, (j-i));
                token::Type tok_type = token::NONE;

                bool is_negative = word_name[0] == '-';

                if (is_negative) {
                    std::string abs_word = word_name.substr(1);
                    if (is_int(abs_word))
                        tok_type = token::INTEGER;

                    else if (is_float(abs_word))
                        tok_type = token::FLOAT;

                    else
                        tok_type = token::WORD;

                } else {
                    if (is_int(word_name))
                        tok_type = token::INTEGER;

                    else if (is_float(word_name))
                        tok_type = token::FLOAT;

                    else
                        tok_type = token::WORD;
                }


                token::Token tmp { tok_type, word_name };
                token_list.push_back(tmp);

                // i = j - 1 so that the next loop starts on the character that broke the word
                i = j - 1;
                break;
            }
        }
    }

    if (paren_count != 0) {
        std::cout << "ERROR: EOF Unbalanced parenthesis\n";
        assert(false);
    }

    if (quote_count != 0) {
        std::cout << "ERROR: EOF Unbalanced quotes\n";
        assert(false);
    }
}

std::vector<token::Token> Parser::get_token_list()
{
    return token_list;
}

void Parser::clear()
{
    token_list.clear();
}

void Parser::print_tokens()
{
    for (auto token : token_list) {
        token.print();
        std::cout << std::endl;
    }
}
    
inline bool Parser::is_int(std::string &str)
{
    if (str.empty() || 
        (((!std::isdigit(str[0]))) && 
         (str[0] != '-') && 
         (str[0] != '+')))

        return false;
    char *ptr;
    std::strtol(str.c_str(), &ptr, 10);
    
    return *ptr == 0;
}

inline bool Parser::is_float(std::string &str)
{
    char *end = nullptr;
    std::strtod(str.c_str(), &end);
    return end != str.c_str() && *end == '\0';
}