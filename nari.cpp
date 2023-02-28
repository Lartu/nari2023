#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <ostream>
#include <stack>
#include <map>
#include <algorithm>
#include <sstream>

#define ERROR_LENGTH 20
#define NUMBER_TYPE 1
#define STRING_TYPE 0
#define STRING_DELIMITER "\""

void *stack_top = 0;
void *stack_bottom = 0;
size_t stack_size = 0;
bool string_was_number = false;
std::string source;
std::string line_feed = "\n";
std::string string_delimiter = STRING_DELIMITER;
std::stack<size_t> execution_stack;
std::stack<size_t> while_stack;
size_t if_count = 0;
std::map<std::string, size_t> words;
std::map<std::string, std::string> str_variables;
std::map<std::string, int64_t> num_variables;
std::string current_word_name;

class NumValue
{
public:
	const uint8_t type;
	void *prev_value;
	void *next_value;
	int64_t value;
	NumValue(int8_t _value) : type(NUMBER_TYPE)
	{
		prev_value = 0;
		next_value = 0;
		value = _value;
	};
};

class StrValue
{
public:
	const uint8_t type;
	void *prev_value;
	void *next_value;
	std::string value;
	StrValue(std::string &_value) : type(STRING_TYPE)
	{
		prev_value = 0;
		next_value = 0;
		value = _value;
	};
};

int64_t parse_number(const std::string &s)
{
	uint_fast64_t final_value = 0;
	bool negative = false;
	for (size_t i = 0; i < s.length(); ++i)
	{
		if (i == 0 && s[i] == '-')
		{
			negative = true;
		}
		else if (std::isdigit(s[i]) == 0)
		{
			string_was_number = false;
			return 0;
		}
		else
		{
			final_value *= 10;
			final_value += s[i] - 48;
		}
	}
	string_was_number = true;
	if (negative)
	{
		final_value *= -1;
	}
	return final_value;
}

std::string to_string(int64_t value)
{
	if (value == 0)
		return "0";
	std::string result = "";
	bool is_negative = false;
	if (value < 0)
	{
		is_negative = true;
		value *= -1;
	}
	while (value > 0)
	{
		uint8_t digit = value % 10;
		char digit_char = digit + 48;
		result = digit_char + result;
		value = value / 10;
	}
	if(is_negative) result = "-" + result;
	return result;
}

void exception(std::string message, std::string &current_command, size_t char_index)
{
	char_index = char_index - current_command.length();
	std::cout << "Accident!" << std::endl;
	std::cout << "Command: " << current_command << std::endl;
	std::cout << "Context: " << (char_index + 1) << std::endl;
	size_t start_index = char_index > ERROR_LENGTH ? char_index - ERROR_LENGTH : 0;
	std::string prefix = "";
	std::string suffix = "";
	if (start_index > 0)
		prefix = "...";
	size_t line_lenght = current_command.length() + ERROR_LENGTH * 2;
	if (start_index + line_lenght < source.length())
	{
		suffix = "...";
	}
	std::string error_line = source.substr(start_index, line_lenght);
	for (size_t i = 0; i < error_line.length(); ++i)
	{
		if (isspace(error_line[i]))
			error_line[i] = ' ';
	}
	std::cout << "Line: " << prefix << error_line << suffix << std::endl;
	std::cout << "      ";
	// TODO que si hay saltos de linea se eliminen
	for (size_t i = 0; i < char_index - start_index + prefix.length(); ++i)
	{
		std::cout << " ";
	}
	for (size_t i = 0; i < current_command.length(); ++i)
	{
		std::cout << "^";
	}
	std::cout << std::endl;
	std::cout << message << std::endl;
	exit(1);
}

void push_value(void *value)
{
	((NumValue *)value)->prev_value = stack_top;
	if (stack_bottom == 0)
	{
		stack_bottom = value;
	}
	if (stack_top != 0)
	{
		((NumValue *)stack_top)->next_value = value;
	}
	stack_top = value;
	++stack_size;
}

void push_number(int64_t value)
{
	NumValue *stack_val = new NumValue(value);
	push_value((void *)stack_val);
}

void push_string(std::string &value)
{
	StrValue *stack_val = new StrValue(value);
	push_value((void *)stack_val);
}

void *pop(std::string &current_command, size_t char_index)
{
	if (stack_size == 0)
		exception("Trying to pop from an empty stack.", current_command, char_index);
	void *value = stack_top;
	stack_top = ((NumValue *)value)->prev_value;
	if (stack_top == 0)
	{
		stack_bottom = 0;
	}
	else
	{
		((NumValue *)stack_top)->next_value = 0;
	}
	--stack_size;
	return value;
}

void *pop_bottom(std::string &current_command, size_t char_index)
{
	if (stack_size == 0)
		exception("Trying to pop bottom from an empty stack.", current_command, char_index);
	void *value = stack_bottom;
	stack_bottom = ((NumValue *)value)->next_value;
	if (stack_bottom == 0)
	{
		stack_top = 0;
	}
	else
	{
		((NumValue *)stack_bottom)->prev_value = 0;
	}
	--stack_size;
	return value;
}

void get_variable(std::string &var_name, std::string &current_command, size_t char_index)
{
	if (num_variables.count(var_name) > 0)
		push_number(num_variables[var_name]);
	else if (str_variables.count(var_name) > 0)
		push_string(str_variables[var_name]);
	else
		exception("Non-existent variable.", current_command, char_index);
}

void set_variable(std::string &var_name, void *value, std::string &current_command, size_t char_index)
{
	uint8_t type = ((NumValue *)value)->type;
	if (type == NUMBER_TYPE)
	{
		if (str_variables.count(var_name) > 0)
		{
			exception("Variable type mismatch.", current_command, char_index);
		}
		else if (num_variables.count(var_name) > 0)
		{
			num_variables[var_name] = ((NumValue *)value)->value;
		}
		else
		{
			num_variables.insert(std::pair<std::string, size_t>(var_name, ((NumValue *)value)->value));
		}
		delete (NumValue *)value;
	}
	else if (type == STRING_TYPE)
	{
		if (num_variables.count(var_name) > 0)
		{
			exception("Variable type mismatch.", current_command, char_index);
		}
		else if (str_variables.count(var_name) > 0)
		{
			str_variables[var_name] = ((StrValue *)value)->value;
		}
		else
		{
			str_variables.insert(std::pair<std::string, std::string>(var_name, ((StrValue *)value)->value));
		}
		delete (StrValue *)value;
	}
	else
	{
		exception("Cannot assign unknown type.", current_command, char_index);
	}
}

void execute_source()
{
	bool comment_mode = false;
	bool while_no_op_mode = false;
	bool if_no_op_mode = false;
	bool string_mode = false;
	bool word_mode = false;
	size_t char_index = 0;
	std::string current_command = "";
	while (char_index < source.length())
	{
		char current_char = source[char_index];
		// std::cout << "CHAR: " << current_char << std::endl;
		if (comment_mode && current_char == '#')
		{
			comment_mode = false;
		}
		else if (string_mode)
		{
			if (current_char == '"')
			{
				push_string(current_command);
				current_command.clear();
				string_mode = false;
			}
			else
			{
				current_command += current_char;
			}
		}
		else if (!comment_mode && !string_mode)
		{
			if (current_char == ' ' || current_char == '\n' || current_char == '\t')
			{
				if (!word_mode && !while_no_op_mode && !if_no_op_mode)
				{
					if (current_command.length() == 0)
					{
						// pass
					}
					else if (current_command == "#")
					{
						comment_mode = true;
					}
					else if (current_command == STRING_DELIMITER)
					{
						string_mode = true;
					}
					else if (current_command == "{")
					{
						word_mode = true;
					}
					else if (current_command == "+")
					{
						if (stack_size < 2)
							exception("Insufficient stack.", current_command, char_index);
						NumValue *value_2 = (NumValue *)pop(current_command, char_index);
						NumValue *value_1 = (NumValue *)pop(current_command, char_index);
						if (value_1->type != NUMBER_TYPE || value_2->type != NUMBER_TYPE)
						{
							exception("Non-numeric values.", current_command, char_index);
						}
						push_number(value_1->value + value_2->value);
						delete value_1;
						delete value_2;
					}
					else if (current_command == "-")
					{
						if (stack_size < 2)
							exception("Insufficient stack.", current_command, char_index);
						NumValue *value_2 = (NumValue *)pop(current_command, char_index);
						NumValue *value_1 = (NumValue *)pop(current_command, char_index);
						if (value_1->type != NUMBER_TYPE || value_2->type != NUMBER_TYPE)
						{
							exception("Non-numeric values.", current_command, char_index);
						}
						push_number(value_1->value - value_2->value);
						delete value_1;
						delete value_2;
					}
					else if (current_command == "*")
					{
						if (stack_size < 2)
							exception("Insufficient stack.", current_command, char_index);
						NumValue *value_2 = (NumValue *)pop(current_command, char_index);
						NumValue *value_1 = (NumValue *)pop(current_command, char_index);
						if (value_1->type != NUMBER_TYPE || value_2->type != NUMBER_TYPE)
						{
							exception("Non-numeric values.", current_command, char_index);
						}
						push_number(value_1->value * value_2->value);
						delete value_1;
						delete value_2;
					}
					else if (current_command == "/")
					{
						if (stack_size < 2)
							exception("Insufficient stack.", current_command, char_index);
						NumValue *value_2 = (NumValue *)pop(current_command, char_index);
						NumValue *value_1 = (NumValue *)pop(current_command, char_index);
						if (value_1->type != NUMBER_TYPE || value_2->type != NUMBER_TYPE)
						{
							exception("Non-numeric values.", current_command, char_index);
						}
						push_number(value_1->value / value_2->value);
						delete value_1;
						delete value_2;
					}
					else if (current_command == "%")
					{
						if (stack_size < 2)
							exception("Insufficient stack.", current_command, char_index);
						NumValue *value_2 = (NumValue *)pop(current_command, char_index);
						NumValue *value_1 = (NumValue *)pop(current_command, char_index);
						if (value_1->type != NUMBER_TYPE || value_2->type != NUMBER_TYPE)
						{
							exception("Non-numeric values.", current_command, char_index);
						}
						push_number((value_1->value % value_2->value + value_2->value) % value_2->value);
						delete value_1;
						delete value_2;
					}
					else if (current_command == ".")
					{
						while (stack_size > 0)
						{
							void *value = pop_bottom(current_command, char_index);
							uint8_t type = ((NumValue *)value)->type;
							if (type == NUMBER_TYPE)
							{
								std::cout << ((NumValue *)value)->value;
							}
							else if (type == STRING_TYPE)
							{
								std::cout << ((StrValue *)value)->value;
							}
							else
							{
								exception("Cannot print unknown type.", current_command, char_index);
							}
						}
					}
					else if (current_command == "=")
					{
						if (stack_size < 2)
							exception("Insufficient stack.", current_command, char_index);
						void *value_2 = pop(current_command, char_index);
						void *value_1 = pop(current_command, char_index);
						if (((NumValue *)value_1)->type != ((NumValue *)value_2)->type)
						{
							push_number(0);
						}
						else if (((NumValue *)value_1)->type == NUMBER_TYPE)
						{
							if (((NumValue *)value_1)->value == ((NumValue *)value_2)->value)
							{
								push_number(1);
							}
							else
							{
								push_number(0);
							}
							delete (NumValue *)value_1;
							delete (NumValue *)value_2;
						}
						else if (((NumValue *)value_1)->type == STRING_TYPE)
						{
							if (((StrValue *)value_1)->value == ((StrValue *)value_2)->value)
							{
								push_number(1);
							}
							else
							{
								push_number(0);
							}
							delete (StrValue *)value_1;
							delete (StrValue *)value_2;
						}
					}
					else if (current_command == "!=")
					{
						if (stack_size < 2)
							exception("Insufficient stack.", current_command, char_index);
						void *value_2 = pop(current_command, char_index);
						void *value_1 = pop(current_command, char_index);
						if (((NumValue *)value_1)->type != ((NumValue *)value_2)->type)
						{
							push_number(0);
						}
						else if (((NumValue *)value_1)->type == NUMBER_TYPE)
						{
							if (((NumValue *)value_1)->value != ((NumValue *)value_2)->value)
							{
								push_number(1);
							}
							else
							{
								push_number(0);
							}
							delete (NumValue *)value_1;
							delete (NumValue *)value_2;
						}
						else if (((NumValue *)value_1)->type == STRING_TYPE)
						{
							if (((StrValue *)value_1)->value != ((StrValue *)value_2)->value)
							{
								push_number(1);
							}
							else
							{
								push_number(0);
							}
							delete (StrValue *)value_1;
							delete (StrValue *)value_2;
						}
					}
					else if (current_command == "<")
					{
						if (stack_size < 2)
							exception("Insufficient stack.", current_command, char_index);
						void *value_2 = pop(current_command, char_index);
						void *value_1 = pop(current_command, char_index);
						if (((NumValue *)value_1)->type != NUMBER_TYPE || ((NumValue *)value_2)->type != NUMBER_TYPE)
						{
							exception("Expecting numeric values.", current_command, char_index);
						}
						if (((NumValue *)value_1)->value < ((NumValue *)value_2)->value)
						{
							push_number(1);
						}
						else
						{
							push_number(0);
						}
						delete (NumValue *)value_1;
						delete (NumValue *)value_2;
					}
					else if (current_command == ">")
					{
						if (stack_size < 2)
							exception("Insufficient stack.", current_command, char_index);
						void *value_2 = pop(current_command, char_index);
						void *value_1 = pop(current_command, char_index);
						if (((NumValue *)value_1)->type != NUMBER_TYPE || ((NumValue *)value_2)->type != NUMBER_TYPE)
						{
							exception("Expecting numeric values.", current_command, char_index);
						}
						if (((NumValue *)value_1)->value > ((NumValue *)value_2)->value)
						{
							push_number(1);
						}
						else
						{
							push_number(0);
						}
						delete (NumValue *)value_1;
						delete (NumValue *)value_2;
					}
					else if (current_command == "<=")
					{
						if (stack_size < 2)
							exception("Insufficient stack.", current_command, char_index);
						void *value_2 = pop(current_command, char_index);
						void *value_1 = pop(current_command, char_index);
						if (((NumValue *)value_1)->type != NUMBER_TYPE || ((NumValue *)value_2)->type != NUMBER_TYPE)
						{
							exception("Expecting numeric values.", current_command, char_index);
						}
						if (((NumValue *)value_1)->value <= ((NumValue *)value_2)->value)
						{
							push_number(1);
						}
						else
						{
							push_number(0);
						}
						delete (NumValue *)value_1;
						delete (NumValue *)value_2;
					}
					else if (current_command == ">=")
					{
						if (stack_size < 2)
							exception("Insufficient stack.", current_command, char_index);
						void *value_2 = pop(current_command, char_index);
						void *value_1 = pop(current_command, char_index);
						if (((NumValue *)value_1)->type != NUMBER_TYPE || ((NumValue *)value_2)->type != NUMBER_TYPE)
						{
							exception("Expecting numeric values.", current_command, char_index);
						}
						if (((NumValue *)value_1)->value >= ((NumValue *)value_2)->value)
						{
							push_number(1);
						}
						else
						{
							push_number(0);
						}
						delete (NumValue *)value_1;
						delete (NumValue *)value_2;
					}
					else if (current_command == "and")
					{
						if (stack_size < 2)
							exception("Insufficient stack.", current_command, char_index);
						void *value_2 = pop(current_command, char_index);
						void *value_1 = pop(current_command, char_index);
						if (((NumValue *)value_1)->type != NUMBER_TYPE || ((NumValue *)value_2)->type != NUMBER_TYPE)
						{
							exception("Expecting numeric values.", current_command, char_index);
						}
						if (((NumValue *)value_1)->value != 0 && ((NumValue *)value_2)->value != 0)
						{
							push_number(1);
						}
						else
						{
							push_number(0);
						}
						delete (NumValue *)value_1;
						delete (NumValue *)value_2;
					}
					else if (current_command == "or")
					{
						if (stack_size < 2)
							exception("Insufficient stack.", current_command, char_index);
						void *value_2 = pop(current_command, char_index);
						void *value_1 = pop(current_command, char_index);
						if (((NumValue *)value_1)->type != NUMBER_TYPE || ((NumValue *)value_2)->type != NUMBER_TYPE)
						{
							exception("Expecting numeric values.", current_command, char_index);
						}
						if (((NumValue *)value_1)->value != 0 || ((NumValue *)value_2)->value != 0)
						{
							push_number(1);
						}
						else
						{
							push_number(0);
						}
						delete (NumValue *)value_1;
						delete (NumValue *)value_2;
					}
					else if (current_command == "lf")
					{
						push_string(line_feed);
					}
					else if (current_command == "sd")
					{
						push_string(string_delimiter);
					}
					else if (current_command == "}")
					{
						if (execution_stack.empty())
							exception("Execution stack empty.", current_command, char_index);
						char_index = execution_stack.top();
						execution_stack.pop();
					}
					else if (current_command == "[")
					{
						while_stack.push(char_index - 2);
					}
					else if (current_command == "!")
					{
						if (stack_size < 1)
							exception("Insufficient stack.", current_command, char_index);
						NumValue *value_1 = (NumValue *)pop(current_command, char_index);
						if (value_1->type != NUMBER_TYPE)
						{
							exception("Non-numeric value.", current_command, char_index);
						}
						if (value_1->value == 0)
							push_number(1);
						else
							push_number(0);
						delete (NumValue *)value_1;
					}
					else if (current_command == ":")
					{
						if (stack_size < 1)
							exception("Insufficient stack.", current_command, char_index);
						NumValue *value_1 = (NumValue *)pop(current_command, char_index);
						if (value_1->type != NUMBER_TYPE)
						{
							exception("Non-numeric value.", current_command, char_index);
						}
						if (value_1->value == 0)
						{
							while_no_op_mode = true;
						}
						delete (NumValue *)value_1;
					}
					else if (current_command == "]")
					{
						if (while_stack.size() == 1)
						{
							char_index = while_stack.top();
						}
						while_stack.pop();
					}
					else if (current_command == "(")
					{
						if (stack_size < 1)
							exception("Insufficient stack.", current_command, char_index);
						NumValue *value_1 = (NumValue *)pop(current_command, char_index);
						if (value_1->type != NUMBER_TYPE)
						{
							exception("Non-numeric value.", current_command, char_index);
						}
						if (value_1->value == 0)
						{
							if_no_op_mode = true;
						}
						++if_count;
						delete (NumValue *)value_1;
					}
					else if (current_command == "|")
					{
						if (if_count == 0)
							exception("Wrong context.", current_command, char_index);
						if_no_op_mode = true;
					}
					else if (current_command == ")")
					{
						if (if_count == 0)
							exception("Wrong context.", current_command, char_index);
						--if_count;
					}
					else if (current_command == "@")
					{
						if (stack_size < 1)
							exception("Insufficient stack.", current_command, char_index);
						StrValue *value_1 = (StrValue *)pop(current_command, char_index);
						if (value_1->type != STRING_TYPE)
						{
							exception("Non-string value.", current_command, char_index);
						}
						std::string var_name = value_1->value;
						delete value_1;
						get_variable(var_name, current_command, char_index);
					}
					else if (current_command == "$")
					{
						if (stack_size < 1)
							exception("Insufficient stack.", current_command, char_index);
						StrValue *value_1 = (StrValue *)pop(current_command, char_index);
						if (value_1->type != STRING_TYPE)
						{
							exception("Non-string value.", current_command, char_index);
						}
						std::string var_name = value_1->value;
						delete value_1;
						void *value = pop(current_command, char_index);
						set_variable(var_name, value, current_command, char_index);
					}
					else if (current_command == "&")
					{
						if (stack_size < 2)
							exception("Insufficient stack.", current_command, char_index);
						void *value_2 = pop(current_command, char_index);
						void *value_1 = pop(current_command, char_index);
						if (((NumValue *)value_1)->type != STRING_TYPE || ((NumValue *)value_2)->type != STRING_TYPE)
						{
							exception("Expecting string values.", current_command, char_index);
						}
						std::string string_value = ((StrValue *)value_1)->value + ((StrValue *)value_2)->value;
						push_string(string_value);
						delete (StrValue *)value_1;
						delete (StrValue *)value_2;
					}
					else if (current_command == "s")
					{
						if (stack_size < 1)
							exception("Insufficient stack.", current_command, char_index);
						void *value_1 = pop(current_command, char_index);
						if (((NumValue *)value_1)->type != NUMBER_TYPE)
						{
							exception("Expecting numeric value.", current_command, char_index);
						}
						std::string string_value = to_string(((NumValue *)value_1)->value);
						push_string(string_value);
						delete (NumValue *)value_1;
					}
					else if (current_command == "n")
					{
						if (stack_size < 1)
							exception("Insufficient stack.", current_command, char_index);
						void *value_1 = pop(current_command, char_index);
						if (((NumValue *)value_1)->type != STRING_TYPE)
						{
							exception("Expecting string value.", current_command, char_index);
						}
						uint64_t value = parse_number(((StrValue *)value_1)->value);
						if (string_was_number)
						{
							push_number(value);
						}else{
							exception("Failed to cast to number.", current_command, char_index);
						}
						delete (NumValue *)value_1;
					}
					else
					{
						uint64_t value = parse_number(current_command);
						if (string_was_number)
						{
							push_number(value);
						}
						else if (words.count(current_command))
						{
							execution_stack.push(char_index);
							char_index = words[current_command];
						}
						else if (current_command.length() > 1 && current_command[0] == '@')
						{
							std::string var_name = current_command.substr(1, current_command.length() - 1);
							get_variable(var_name, current_command, char_index);
						}
						else if (current_command.length() > 1 && current_command[0] == '$')
						{
							std::string var_name = current_command.substr(1, current_command.length() - 1);
							void *value = pop(current_command, char_index);
							set_variable(var_name, value, current_command, char_index);
						}
						else
						{
							exception("Unknown command.", current_command, char_index);
						}
					}
				}
				else if (word_mode)
				{
					if (current_command == "}")
					{
						word_mode = false;
						current_word_name.clear();
					}
					else if (current_word_name.empty())
					{
						current_word_name = current_command;
						words.insert(std::pair<std::string, size_t>(current_word_name, char_index));
					}
				}
				else if (while_no_op_mode)
				{
					if (current_command == "]")
					{
						if (while_stack.size() == 1)
						{
							while_no_op_mode = false;
						}
						while_stack.pop();
					}
					else if (current_command == "[")
					{
						while_stack.push(char_index);
					}
				}
				else if (if_no_op_mode)
				{
					if (current_command == ")")
					{
						if (if_count == 1)
						{
							if_no_op_mode = false;
						}
						--if_count;
					}
					else if (current_command == "(")
					{
						++if_count;
					}
					else if (current_command == "|")
					{
						if (if_count == 1)
						{
							if_no_op_mode = false;
						}
					}
				}
				current_command.clear();
			}
			else
			{
				current_command += current_char;
			}
		}
		++char_index;
	}
}

void load_source(const std::string &filename)
{
	std::ifstream source_file;
	source_file.open(filename);
	if (!source_file.good())
	{
		std::cout << "Failed to load '" << filename << "'." << std::endl;
		exit(1);
	}
	std::string line;
	while (std::getline(source_file, line))
	{
		source += line + " ";
	}
}

int main(int argc, char **argv)
{
	if (argc == 1)
	{
		std::cout << "Filename expected." << std::endl;
		return 1;
	}
	load_source((std::string)argv[1]);
	execute_source();
}