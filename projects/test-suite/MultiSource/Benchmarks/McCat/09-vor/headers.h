#define ever (;;)
#define true 1
#define false 0
#define bool int
#define l 0
#define r 1

/* vor.c */
void clean_up();
void getpoint(int x,int y);
int set_node();
int do_quit();
void build_menu(void);
void get_file();
void add_point(Point p);
Point compute_v(CHpoints *P);
void add_infinit_points_to_K(CHpoints *S);
void add_edge(int p1,int p2);
void draw_sec(CHpoints *p);
CHpoints *maximize_radius_and_angle(CHpoints *S);
void alg2();
void construct_vor();

/* ch.c */
bool empty();
bool point_equal(Point p1,Point p2);
int determinant(Point p1, Point p2, Point p3);
bool visible(int direction,Point p1,Point p2,Point p3);
CHpoints *get_points_on_hull(DCEL_segment *left,DCEL_segment *right);
void add_segments(DCEL_segment *n,DCEL_segment *act,DCEL_segment *first,
		  int direction);
CHpoints *construct_ch();

/* splay.c */
void traverse(splay_node *root);
void free_tree(splay_node *root);
splay_node *init(void);
void *insert(splay_node **root, Point p);
Point delete_min(splay_node **root);

/* splay2.c */
void CHtraverse(CHsplay_node *root);
void CHfree_tree(CHsplay_node *root);
CHsplay_node *CHinit(void);
void *CHinsert(CHsplay_node **root, CHpoints *p);
CHpoints *CHdelete_max(CHsplay_node **root);
void CHdelete(CHsplay_node **root, key key);

/* pointlist.c */
void point_list_insert(CHpoints **PL, Point p);
CHpoints *before(CHpoints *P);
CHpoints *next(CHpoints *P);
double angle(CHpoints *p1, CHpoints *p2, CHpoints *p3);
void point_list_print(CHpoints *PL);
void number_points(CHpoints *PL);
CHpoints *remove_points(CHpoints *PL);

/* intersect.c */
dpoint midpoint(Point p1, Point p2);
Point vector(Point p1, Point p2);
int length2(Point p1, Point p2);
double calculate_c(Point normalvector,dpoint midpoint);
dpoint intersect(Point n1, Point n2, double c1, double c2);
dpoint centre(Point p1, Point p2, Point p3);
double radius2(Point p,dpoint centre);
