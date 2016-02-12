#define STATE(x)      s_##x :
#define GOTOSTATE(x)  goto s_##x
#define NEXTSTATE(POINTER)			\
	goto *dtable[(uint8_t) *++(POINTER)];
#define POINTEDSTATE(POINTER)			\
	goto *dtable[(uint8_t) *(POINTER)]
#define MOVEBY(POINTER, TYPE, AMOUNT)				\
	POINTER = ((char *) (((TYPE *) POINTER) + (AMOUNT)))
#define COPY(VARIABLE, POINTER)		\
	VARIABLE = *((typeof(VARIABLE) *) POINTER)
#define POPSET(VARIABLE, POINTER)			\
	do {						\
		COPY(VARIABLE, POINTER);		\
		MOVEBY(POINTER, typeof(VARIABLE), 1);	\
	} while (0)

#define POPSETSTRING(VARIABLE, POINTER)					\
	do {								\
	        VARIABLE ## _size = strlen(POINTER) + 1;		\
		VARIABLE = malloc(VARIABLE ## _size);			\
		strcpy(VARIABLE, POINTER);				\
	        POINTER += VARIABLE ## _size;				\
	} while (0)
#define POP(TYPE, POINTER)						\
	({								\
		TYPE temp;						\
		COPY(temp, POINTER);					\
		MOVEBY(POINTER, TYPE, 1);				\
		temp;							\
	})
