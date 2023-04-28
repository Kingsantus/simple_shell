#include "main.h"

/**
* get_args - Gets a command from standard input.
*
* Return: If an error occurs - NULL.
*/
char *get_args(char *line, int *exe_ret)
{
	size_t n = 0;
	ssize_t read;
	char *prompt = "cisfun$ ";

	if (line)
		free(line);
	read = _getline(&line, &n, STDIN_FILENO);
	if (read == -1)
		return (NULL);
	if (read == 1)
	{
		hist++;
		if (isatty(STDIN_FILENO))
			write(STDOUT_FILENO, prompt, 2);
		return (get_args(line, exe_ret));
	}
	
	line[read - 1] = '\0';
	variable_replacement(&line, exe_ret);
	handle_line(&line, read);

	return (line);
}

/**
* args_call - Partitions operators from commands and calls them.
* @exe_ret: The return value of the parent process' last execd command.
*
* Return: The return value of the last execd command.
*/
int args_call(char **args, char **front, int *exe_ret)
{
	int ret, index;

	if (!args[0])
		return (*exe_ret);
	for (index = 0; args[index]; index++)
	{
		if (_strncmp(args[index], "||", 2) == 0)
		{
			free(args[index]);
			args[index] = NULL;
			args = replace_aliases(args);
			ret = args_exe(args, front, exe_ret);
			if (*exe_ret != 0)
			{
				args = &args[++index];
				index = 0;
			}
			else
			{
				for (index++; args[index]; index++)
					free(args[index]);
				return (ret);
			}
		}
		else if (_strncmp(args[index], "&&", 2) == 0)
		{
			free(args[index]);
			args[index] = NULL;
			args = replace_aliases(args);
			ret = args_exe(args, front, exe_ret);
			if (*exe_ret == 0)
			{
				args = &args[++index];
				index = 0;
			}
			else
			{
				for (index++; args[index]; index++)
					free(args[index]);
				return (ret);
			}
		}
	}
	args = replace_aliases(args);
	ret = args_exe(args, front, exe_ret);
	return (ret);
}

/**
* args_exe - Calls the execution of a command.
* Return: The return value of the last execd command.
*/
int args_exe(char **args, char **front, int *exe_ret)
{
	int ret, i;
	int (*builtin)(char **args, char **front);

	builtin = get_builtin(args[0]);

	if (builtin)
	{
		ret = builtin(args + 1, front);
		if (ret != EXIT)
			*exe_ret = ret;
	}
	else
	{
		*exe_ret = exec(args, front);
		ret = *exe_ret;
	}

	hist++;

	for (i = 0; args[i]; i++)
		free(args[i]);

	return (ret);
}

/**
* handle_args - Gets, calls, and runs the execution of a command.
* @exe_ret: The return value of the parent process' last execd command.
*
* Return: If an end-of-file is read - END_OF_FILE (-2).
*/
int handle_args(int *exe_ret)
{
	int ret = 0, index;
	char **args, *line = NULL, **front;

	line = get_args(line, exe_ret);
	if (!line)
		return (END_OF_FILE);

	args = _strtok(line, " ");
	free(line);
	if (!args)
		return (ret);
	if (check_args(args) != 0)
	{
		*exe_ret = 2;
		free_args(args, args);
		return (*exe_ret);
	}
	front = args;

	for (index = 0; args[index]; index++)
	{
		if (_strncmp(args[index], ";", 1) == 0)
		{
			free(args[index]);
			args[index] = NULL;
			ret = args_call(args, front, exe_ret);
			args = &args[++index];
			index = 0;
		}
	}
	if (args)
		ret = args_call(args, front, exe_ret);

	free(front);
	return (ret);
}

/**
* check_args - Checks if there are any leading ';', ';;', '&&', or '||'.
*
* Return: If a ';', '&&', or '||' is placed at an invalid position - 2.
*/
int check_args(char **args)
{
	size_t i;
	char *cur, *nex;

	for (i = 0; args[i]; i++)
	{
		cur = args[i];
		if (cur[0] == ';' || cur[0] == '&' || cur[0] == '|')
		{
			if (i == 0 || cur[1] == ';')
				return (error_create(&args[i], 2));
			nex = args[i + 1];
			if (nex && (nex[0] == ';' || nex[0] == '&' || nex[0] == '|'))
				return (error_create(&args[i + 1], 2));
		}
	}
	return (0);
}
