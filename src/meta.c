#if (CPU_ && TYP_)

#define MAX_PARAM_COUNT 256

typedef struct Line Line;
struct Line {
	Line *next;
	Line *prev;

	String8 str;
	L1 param_count;
	String8 params[MAX_PARAM_COUNT];
};

typedef struct Line_List Line_List;
struct Line_List {
	Line *first;
	Line *last;
	L1 count;
};

#endif

#if (CPU_ && ROM_)

Internal Line *parse_line(Arena *arena, String8 str) {
	Line *line = push_array(arena, Line, 1);
	line->str = str;

	I1 in_backtick = 0;
	L1 param_begin_idx = L1_MAX;

	for EachIndex(i, str.len) {
		char c = str.str[i];

		if (c == ' ' || c == '\t') {
			continue;
		}

		if (c == '`') {
			in_backtick = !in_backtick;
		} else if (in_backtick) {
			continue;
		}
	
		if ((c == ',' || c == '\n') && param_begin_idx != L1_MAX) {
			if (line->param_count < MAX_PARAM_COUNT) {
				String8 param = {
					.str = str.str + param_begin_idx,
					.len = i - param_begin_idx,
				};
				if (param.str[0] == '`') {
					param.str += 1;
					param.len -= 1;
				}
				if (param.len > 0 && param.str[param.len-1] == '`') {
					param.len -= 1;
				}
				line->params[line->param_count] = param;
				line->param_count += 1;
			} else {
				printf("meta.c: MAX PARAM COUNT FOR LINE REACHED.\n");
			}
			param_begin_idx = L1_MAX;
		} else if (param_begin_idx == L1_MAX) {
			param_begin_idx = i;
		}
	}

	if (in_backtick) {
		printf("meta.c: Backtick not closed.\n");
		line->param_count = 0;
	}

	return line;
}

Global L1 final_header_length = 0;
Global L1 final_impl_length = 0;
Global String8 final_header = {0};
Global String8 final_impl = {0};

Internal void lane(Arena *arena) {

	////////////////////////////////
	//~ kti: Parse

	String8 content = os_read_entire_file(arena, Str8_("./src/ui.meta"));
	Line_List lines = {0};
	Range range = lane_range(content.len);
	L1 line_start = 0;
	for EachInRange(i, range) {
		if (content.str[i] == '\n') {
			line_start = i;
			while (line_start > 0 && content.str[line_start-1] != '\n') {
				line_start -= 1;
			}

			String8 str = {
				.str = content.str + line_start,
				.len = i - line_start + 1,
			};
			Line *line = parse_line(arena, str);
			DLLPushBack(lines.first, lines.last, line);
			lines.count += 1;
			line_start = i + 1;
		}
	}

	////////////////////////////////
	//~ kti: Print parse result.
#if 0
	for EachIndex(i, lane_count()) {
		if (i == lane_idx()) {

			for (Line *line = lines.first; line != 0; line = line->next) {
				printf("LINE: %.*s", (I1)line->str.len, line->str.str);
				
				for EachIndex(param_idx, line->param_count) {
					String8 param = line->params[param_idx];
					printf("  PARAM: %.*s\n", (I1)param.len, param.str);
				}
			}

		}
		lane_sync();
	}
#endif

	////////////////////////////////
	//~ kti: Generate

	Temp_Arena scratch = scratch_begin(0, 0);

	String8_List header_strings = {0};
	String8_List impl_strings = {0};
	String8_List stacks_strings = {0};
	String8_List reset_stacks_strings = {0};
	String8_List init_nil_stacks_strings = {0};
	String8_List auto_pop_stacks_strings = {0};

	String8 stacks_macro_def = Str8_("#define UIStacks struct {\\\n");
	String8 reset_stacks_macro_def = Str8_("#define UIResetStacks() {\\\n");
	String8 init_nil_stacks_macro_def = Str8_("#define UIInitStackNils() {\\\n");
	String8 auto_pop_stacks_macro_def = Str8_("#define UIAutoPopStacks() {\\\n");
	String8 macro_end = Str8_("}\n");
	if (lane_idx() == 0) {
		str8_list_push(scratch.arena, &stacks_strings, stacks_macro_def);
		str8_list_push(scratch.arena, &reset_stacks_strings, reset_stacks_macro_def);
		str8_list_push(scratch.arena, &init_nil_stacks_strings, init_nil_stacks_macro_def);
		str8_list_push(scratch.arena, &auto_pop_stacks_strings, auto_pop_stacks_macro_def);
	}

	for (Line *line = lines.first; line != 0; line = line->next) {
		if (line->param_count == 4) {
			String8 node_type = str8f(scratch.arena, "UI_%.*s_Node", (I1)line->params[0].len, line->params[0].str);
			String8 stack_type = str8f(scratch.arena, "UI_%.*s_Stack", (I1)line->params[0].len, line->params[0].str);
			String8 name = push_str8_copy(scratch.arena, line->params[1]);
			String8 type = push_str8_copy(scratch.arena, line->params[2]);
			String8 default_value = push_str8_copy(scratch.arena, line->params[3]);
			String8 pascal_case_name = push_str8_copy(scratch.arena, line->params[0]);

			str8_list_pushf(scratch.arena, &header_strings, R"(
typedef struct %1$s %1$s;
struct %1$s { %1$s *next; %4$s value; };
typedef struct %2$s %2$s;
struct %2$s { %1$s *top; %1$s *free; I1 auto_pop; };
Internal void ui_push_%3$s(%4$s value);
Internal %4$s ui_pop_%3$s(void);
Internal void ui_set_next_%3$s(%4$s value);
Internal %4$s ui_top_%3$s(void);
)", node_type.str, stack_type.str, name.str, type.str);

			str8_list_pushf(scratch.arena, &impl_strings, R"(
Internal void ui_push_%3$s(%4$s value) {
	%2$s *stack = &ui_state->%3$s_stack;
	%1$s *node = stack->free;
	if (node == 0) { node = push_array(ui_build_arena(), %1$s, 1); }
	else { SLLStackPop(stack->free); }
	node->value = value;
	SLLStackPush(stack->top, node);
	stack->auto_pop = 0;
}
Internal %4$s ui_pop_%3$s(void) {
	%2$s *stack = &ui_state->%3$s_stack;
	%1$s *popped_node = stack->top;
	if (popped_node != &ui_state->nil_%3$s) {
		SLLStackPop(stack->top);
		SLLStackPush(stack->free, popped_node);
		stack->auto_pop = 0;
	}
	return popped_node->value;
}
Internal void ui_set_next_%3$s(%4$s value) {
	ui_push_%3$s(value);
	ui_state->%3$s_stack.auto_pop = 1;
}
Internal %4$s ui_top_%3$s(void) {
	return ui_state->%3$s_stack.top->value;
}
#define UI_%5$s(v) DeferLoop(ui_push_%3$s(v), ui_pop_%3$s())
)", node_type.str, stack_type.str, name.str, type.str, pascal_case_name.str);

			str8_list_pushf(scratch.arena, &stacks_strings,
					"	%2$s %3$s_stack;\\\n"
					"	%1$s nil_%3$s; \\\n",
					node_type.str, stack_type.str, name.str, type.str);

			str8_list_pushf(scratch.arena, &reset_stacks_strings,
					"	ui_state->%3$s_stack.top = &ui_state->nil_%3$s; ui_state->%3$s_stack.free = 0; ui_state->%3$s_stack.auto_pop = 0;\\\n",
					node_type.str, stack_type.str, name.str, type.str);

			str8_list_pushf(scratch.arena, &init_nil_stacks_strings,
					"	ui_state->nil_%3$s.value = %5$s;\\\n",
					node_type.str, stack_type.str, name.str, type.str, default_value.str);

			str8_list_pushf(scratch.arena, &auto_pop_stacks_strings,
					"	if (ui_state->%3$s_stack.auto_pop) { ui_pop_%3$s(); ui_state->%3$s_stack.auto_pop = 0; }\\\n",
					node_type.str, stack_type.str, name.str, type.str);
		}
	}

	if (lane_idx() == lane_count()-1) {
		str8_list_push(scratch.arena, &stacks_strings, macro_end);
		str8_list_push(scratch.arena, &reset_stacks_strings, macro_end);
		str8_list_push(scratch.arena, &init_nil_stacks_strings, macro_end);
		str8_list_push(scratch.arena, &auto_pop_stacks_strings, macro_end);
	}

	String8 header_result = str8_list_join(arena, &header_strings);
	String8 impl_result = str8_list_join(arena, &impl_strings);
	String8 stacks_result = str8_list_join(arena, &stacks_strings);
	String8 reset_stacks_result = str8_list_join(arena, &reset_stacks_strings);
	String8 init_nil_stacks_result = str8_list_join(arena, &init_nil_stacks_strings);
	String8 auto_pop_stacks_result = str8_list_join(arena, &auto_pop_stacks_strings);

	atomic_add_L1(&final_header_length, header_result.len+stacks_result.len+reset_stacks_result.len+init_nil_stacks_result.len+auto_pop_stacks_result.len);
	atomic_add_L1(&final_impl_length, impl_result.len);

	scratch_end(scratch);

	//- kti: Allocate result header and impl strings.
	lane_sync();
	if (lane_idx() == 0) {
		String8 notice = Str8_("// IMPORTANT: DO NOT EDIT! This file was generated by meta.c\n");

		String8 header_top = Str8_("#pragma once\n");
		final_header.str = push_array(arena, B1, final_header_length + notice.len + header_top.len);
		memmove(final_header.str+final_header.len, notice.str, notice.len);
		final_header.len += notice.len;

		memmove(final_header.str+final_header.len, header_top.str, header_top.len);
		final_header.len += header_top.len;
		
		final_impl.str = push_array(arena, B1, final_impl_length + notice.len);
		memmove(final_impl.str+final_impl.len, notice.str, notice.len);
		final_impl.len += notice.len;
	}
	lane_sync();

	//- kti: Write types and functions.
	for EachIndex(i, lane_count()) {
		if (i == lane_idx()) {
			memmove(final_header.str+final_header.len, header_result.str, header_result.len);
			memmove(final_impl.str+final_impl.len, impl_result.str, impl_result.len);
			final_header.len += header_result.len;
			final_impl.len += impl_result.len;
		}
		lane_sync();
	}

	//- kti: Write UIStacks macro.
	for EachIndex(i, lane_count()) {
		if (i == lane_idx()) {
			memmove(final_header.str+final_header.len, stacks_result.str, stacks_result.len);
			final_header.len += stacks_result.len;
		}
		lane_sync();
	}
	lane_sync();

	//- kti: Write UIResetStacks macro.
	for EachIndex(i, lane_count()) {
		if (i == lane_idx()) {
			memmove(final_header.str+final_header.len, reset_stacks_result.str, reset_stacks_result.len);
			final_header.len += reset_stacks_result.len;
		}
		lane_sync();
	}

	//- kti: Write UIInitNil macro.
	for EachIndex(i, lane_count()) {
		if (i == lane_idx()) {
			memmove(final_header.str+final_header.len, init_nil_stacks_result.str, init_nil_stacks_result.len);
			final_header.len += init_nil_stacks_result.len;
		}
		lane_sync();
	}

	//- kti: Write UIAutoPopStacks macro.
	for EachIndex(i, lane_count()) {
		if (i == lane_idx()) {
			memmove(final_header.str+final_header.len, auto_pop_stacks_result.str, auto_pop_stacks_result.len);
			final_header.len += auto_pop_stacks_result.len;
		}
		lane_sync();
	}

	lane_sync();

	if (lane_idx() == 0) {
		os_write_entire_file(Str8_("./src/ui.meta.h"), final_header.str, final_header.len);
		os_write_entire_file(Str8_("./src/ui.meta.c"), final_impl.str, final_impl.len);
	}

}

#endif
