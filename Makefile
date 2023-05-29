SRCS	=	srcs/main.c \
		srcs/utils.c \

INCL		= includes/main.h
OBJS		= ${SRCS:.c=.o}
NAME		= ft_malcolm
LIBFT_DIR	= libft/
LIBFT		= ${LIBFT_DIR}libft.a
CC			= gcc
CCFLAGS		= -Wall -Wextra -Werror -I ${INCL} -I ${LIBFT_DIR} #-fsanitize=address
#/Users/$(USER)/.brew
%.o: %.c
	$(CC) $(CCFLAGS) -I/usr/local/opt/readline/include -c $< -o $@

all:	${LIBFT} ${INCL} ${NAME}

${NAME}:	${OBJS} ${INCL}
					$(CC) $(CCFLAGS) -o $(NAME) $(OBJS) $(LIBFT) -lreadline -L /usr/local/opt/readline/lib

${LIBFT}:
			@make -C./libft

clean:
			rm -f ${OBJS}
			@make clean -C ./libft

fclean:	clean
			rm -f ${NAME}
			rm -f ${LIBFT}

re:		fclean all

.PHONY:	all clean fclean re
