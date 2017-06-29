NAME		=	gomoku

FLAGS		=	-std=c++11 -Wall -Werror -Wextra -O3

RFLAGS		=	-L/Users/$(USER)/.brew/lib -Wl,-rpath,/Users/$(USER)/.brew/lib -lsfml-system -lsfml-graphics -lsfml-window -lboost_serialization

SRCDIR		=	srcs/

INCDIR		=	-I includes/ -I/Users/$(USER)/.brew/include

OBJDIR		=	objs/

SRC			=	main.cpp \
				Game.cpp \
				GUI.cpp \
				GUIManager.cpp \
				PlayerColor.cpp \
				TextureManager.cpp \

SRCS =	$(addprefix $(SRCDIR), $(SRC))

OBJ	= $(addprefix $(OBJDIR), $(SRC:.cpp=.o))

all:		$(NAME)

$(OBJDIR):
	@mkdir -p $(OBJDIR)
	@mkdir -p $(dir $(OBJ))

$(OBJDIR)%.o : $(SRCDIR)%.cpp | $(OBJDIR)
	g++ $(FLAGS) -c $< -o $@ $(INCDIR) 


$(NAME):	$(OBJDIR) $(OBJ)
	g++ $(FLAGS) -o $(NAME) $(OBJ) $(RFLAGS)

clean:
	if [ -d $(OBJDIR) ]; then rm -r $(OBJDIR); fi

fclean:	clean
	@rm -f $(NAME)

re:	fclean all


.PHONY: fclean clean re 

.SILENT: clean
