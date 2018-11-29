#include "src/Builtins.h"

typedef struct class_DoesntMatter *class_DoesntMatter;
typedef struct obj_DoesntMatter *obj_DoesntMatter;

struct class_DoesntMatter {
	class_Obj super;
	obj_Nothing (*zlarpo) (obj_DoesntMatter);
};

struct obj_DoesntMatter {
	class_DoesntMatter class;
	obj_DoesntMatter field_this;
};

// DoesntMatter Methods
obj_DoesntMatter new_DoesntMatter() {
	obj_Boolean this_w;
	obj_Int this_x;
	obj_Int this_y;
	obj_String this_z;
}

obj_Nothing DoesntMatter_method_zlarpo(obj_DoesntMatter this, obj_UNKNOWN arg_b, obj_String arg_blem, obj_UNKNOWN arg_c, obj_UNKNOWN arg_end, obj_UNKNOWN arg_g, obj_UNKNOWN arg_x, obj_UNKNOWN arg_z) {
	obj_UNKNOWN local_b;
	obj_String local_blem;
	obj_UNKNOWN local_c;
	obj_UNKNOWN local_end;
	obj_UNKNOWN local_g;
	obj_UNKNOWN local_x;
	obj_UNKNOWN local_z;
}

// main method - it's the end!
int main(int argc, char *argv[]) {
	obj_String u = str_literal("good world");
	obj_Boolean w = lit_true;
	obj_Boolean w = lit_false;
	obj_Int x = int_literal(20);
	obj_Int y = int_literal(20);
	obj_Int z = x.PLUS(int_literal(5));
	// adding two str consts, need a temporary variable
	obj_String tempStr = str_literal("goodbye");
	obj_String z = tempStr.PLUS(str_literal("hello"));
	obj_Int z = x.PLUS(y);
	// adding two int consts, need a temporary variable
	obj_Int tempInt = int_literal(5);
	obj_Int z = tempInt.PLUS(int_literal(5));
	obj_DoesntMatter b = new_DoesntMatter(int_literal(5), y, str_literal("bye"), lit_true);

	return 0;
}
