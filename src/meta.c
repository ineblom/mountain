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

	L1 header_length = 0;
	L1 impl_length = 0;
	String8_List header_strings = {0};
	String8_List impl_strings = {0};

	for (Line *line = lines.first; line != 0; line = line->next) {
		if (line->param_count == 4) {
			String8 node_type = str8f(scratch.arena, "UI_%.*s_Node", (I1)line->params[0].len, line->params[0].str);
			String8 stack_type = str8f(scratch.arena, "UI_%.*s_Stack", (I1)line->params[0].len, line->params[0].str);
			String8 name = push_str8_copy(scratch.arena, line->params[1]);
			String8 type = push_str8_copy(scratch.arena, line->params[2]);
			String8 default_value = push_str8_copy(scratch.arena, line->params[3]);

			String8 generated_header = str8f(scratch.arena, R"(
typedef struct %1$s %1$s;
struct %1$s { %1$s next; %4$s value; };
typedef struct %2$s %2$s;
struct %2$s { %1$s *top; %1$s *free; I1 auto_pop; };
Internal void ui_push_%3$s(%4$s value);
Internal void ui_pop_%3$s(void);
Internal void set_next_%3$s(%4$s value);
Internal %4$s ui_top_%3$s(void);
)", node_type.str, stack_type.str, name.str, type.str);

			String8 generated_impl = str8f(scratch.arena, R"(
Internal void ui_push_%3$s(%4$s value) {
	%2$s *stack = &ui_state->%3$s_stack;
	%1$s *node = stack->free;
	if (node == 0) { node = push_array(ui_build_arena(), %1$s, 1); }
	else { SLLStackPop(stack->free); }
	node->value = value;
	SLLStackPush(stack->top, node);
	stack->auto_pop = 0;
}
Internal void ui_pop_%3$s(void) {
	%2$s *stack = &ui_state->%3$s_stack;
	%1$s *popped_node = stack->top;
	if (popped_node != &ui_state->nil_%3$s) {
		SLLStackPop(stack->top);
		SLLStackPush(stack->free, popped_node);
		stack->auto_pop = 0;
	}
}
Internal void set_next_%3$s(%4$s value) {
	ui_push_%3$s(value);
	ui_state->%3$s_stack.auto_pop = 0;
}
Internal %4$s ui_top_%3$s(void) {
	return ui_state->%3$s_stack.top->value;
}
)", node_type.str, stack_type.str, name.str, type.str);

			String8_Node *header_node = push_array(scratch.arena, String8_Node, 1);
			header_node->value = generated_header;

			String8_Node *impl_node = push_array(scratch.arena, String8_Node, 1);
			impl_node->value = generated_impl;

			DLLPushBack(header_strings.first, header_strings.last, header_node);
			DLLPushBack(impl_strings.first, impl_strings.last, impl_node);

			header_length += generated_header.len;
			impl_length += generated_impl.len;
		}
	}

	atomic_add_L1(&final_header_length, header_length);
	atomic_add_L1(&final_impl_length, impl_length);

	String8 header_result = {
		.len = 0,
		.str = push_array(arena, B1, header_length),
	};
	String8 impl_result = {
		.len = 0,
		.str = push_array(arena, B1, impl_length),
	};
	for (String8_Node *n = header_strings.first; n != 0; n = n->next) {
		memmove(header_result.str+header_result.len, n->value.str, n->value.len);
		header_result.len += n->value.len;
	}
	for (String8_Node *n = impl_strings.first; n != 0; n = n->next) {
		memmove(impl_result.str+impl_result.len, n->value.str, n->value.len);
		impl_result.len += n->value.len;
	}

	scratch_end(scratch);

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

	for EachIndex(i, lane_count()) {
		if (i == lane_idx()) {
			memmove(final_header.str+final_header.len, header_result.str, header_result.len);
			memmove(final_impl.str+final_impl.len, impl_result.str, impl_result.len);
			final_header.len += header_result.len;
			final_impl.len += impl_result.len;
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
