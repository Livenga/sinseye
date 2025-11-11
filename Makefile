CC = gcc
OBJDIR = objs
PRJC = sinseye
SRC =\
	 src/main.c \
	 src/camera.c \
	 src/xioctl.c \
	 src/v4l2_descriptions.c


OBJS = $(addprefix $(OBJDIR)/,$(patsubst %.c,%.o,$(SRC)))
OBJDIRS = $(sort $(addprefix $(OBJDIR)/,$(dir $(SRC))))

DEFINES = \
		  -D__DEBUG__
LINKS = \
		-lm -lgpiod -lv4l2
FLAGS = -g


.PHONY : default clean


default:
	@[ -d "$(OBJDIR)" ] || mkdir -v $(OBJDIR)
	@[ -d "$(OBJDIRS)" ] || mkdir -v $(OBJDIRS)
	make $(PRJC)

$(PRJC):$(OBJS)
	$(CC) -o $@ $^ \
		$(FLAGS) \
		$(LINKS)

$(OBJDIR)/%.o:%.c
	$(CC) -o $@ -c $< \
		$(FLAGS) \
		$(DEFINES)

all:
	make clean default

clean:
	@[ ! -d $(OBJDIR) ] || rm -rv $(OBJDIR)
	@[ ! -f $(PRJC) ] || rm -v $(PRJC)
