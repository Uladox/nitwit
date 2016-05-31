struct ntwt_list {
	void *next;
};

#define NTWT_LIST(LIST)				\
	((struct ntwt_list *) (LIST))
#define NTWT_LIST_NEXT(LIST)			\
	((typeof(LIST)) (NTWT_LIST(LIST)->next))
#define NTWT_LIST_CONS(LIST, END)		\
	(NTWT_LIST(LIST)->next = (END))
#define NTWT_NEXT_REF(LIST)			\
	((typeof(LIST) *) &NTWT_LIST(LIST)->next)
#define ntwt_foreach(LIST)			\
	for (; LIST; LIST = NTWT_LIST_NEXT(LIST))
