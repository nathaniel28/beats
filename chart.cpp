#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>

#include "chart.hpp"

Column::Column() : notes(0) {
	holding_note = nullptr;
	note_index = 0;
	drop_index = 0;
}

void Column::emit_verts(int64_t t0, int64_t t1, int column_height, int note_width, int note_height, int column_offset, std::vector<Point> &points, std::vector<uint32_t> &indices) {
	int i = note_index;
	int max = notes.size();
	while (i < max && static_cast<int64_t>(notes[i].timestamp) < t1) {
		if (static_cast<int64_t>(notes[i].timestamp + notes[i].hold_duration) < t0) {
			// we can do this because notes are kept ordered by time
			note_index = i; // next time, don't bother with notes before this
		} else if (notes[i].active || notes[i].hold_duration > 0) { // we always draw hold notes
			int32_t x0 = note_width * column_offset;
			int32_t x1 = x0 + note_width;
			int32_t y1 = (column_height * (static_cast<int64_t>(notes[i].timestamp) - t0)) / (t1 - t0);
			int32_t y0;
			if (notes[i].hold_duration)
				y0 = y1 + (column_height * static_cast<int64_t>(notes[i].hold_duration)) / (t1 - t0);
			else
				y0 = y1 + note_height;
			uint32_t sz = points.size();
			indices.emplace(indices.end(), sz + 0);
			indices.emplace(indices.end(), sz + 1);
			indices.emplace(indices.end(), sz + 2);
			indices.emplace(indices.end(), sz + 0);
			indices.emplace(indices.end(), sz + 2);
			indices.emplace(indices.end(), sz + 3);
			points.emplace(points.end(), x0, y0);
			points.emplace(points.end(), x0, y1);
			points.emplace(points.end(), x1, y1);
			points.emplace(points.end(), x1, y0);
		}
		i++;
	}
}

std::pair<Note *, int64_t> Column::close_note(uint64_t time, uint64_t threshhold) {
	int max = notes.size();
	if (max == 0)
		return {nullptr, 0};
	uint64_t t_min = time > threshhold ? time - threshhold : 0;
	int i = note_index;
	Note *least = nullptr;
	while (i >= 0 && notes[i].timestamp >= t_min) {
		if (notes[i].active)
			least = &notes[i];
		i--;
	}
	if (least) {
		return {least, time - least->timestamp};
	}
	uint64_t t_max = time + threshhold;
	i = note_index;
	while (i < max && notes[i].timestamp <= t_max) {
		if (notes[i].timestamp >= t_min && notes[i].active) {
			return {&notes[i], time - notes[i].timestamp};
		}
		i++;
	}
	return {nullptr, 0};
}

int Column::drop_before(int64_t time) {
	int max = notes.size();
	int dropped = 0;
	while (drop_index < max && static_cast<int64_t>(notes[drop_index].timestamp) < time) {
		// it took restraint to avoid dropped += notes[drop_index].active
		if (notes[drop_index].active)
			dropped++;
		drop_index++;
	}
	return dropped;
}

Chart::Chart(uint32_t col_height, uint32_t note_width_, uint32_t note_height_) : columns(0), points(512), indices(768), last_press(0), holding_columns(0) {
	column_height = col_height + note_height_;
	note_width = note_width_;
	note_height = note_height_;
}

// expects a file with the following format:
// magic (8 bytes), version (4 bytes), column count (4 bytes)
// followed by column count number of
//   column index (4 bytes), note count (4 bytes)
//   followed by note count number of
//     timestamp (4 bytes), hold duration (4 bytes)
int Chart::deserialize(std::istream &is) {
#define READ_(stream, dat, sz) stream.read(reinterpret_cast<char *>(dat), sz)
	is.exceptions(std::ostream::failbit | std::ostream::badbit);
	try {
		// eventually, use endian.h to be extra standard, but for now,
		// assume little endian
		struct {
			uint64_t magic;
			uint32_t version;
			uint32_t column_count;
		} header; // better be packed right!
		READ_(is, &header, sizeof(header));
		if (header.magic != Chart::magic) {
			std::cerr << "invalid magic " << header.magic << '\n';
			return -1;
		}
		if (header.version != Chart::version) {
			std::cerr << "invalid version " << header.version << '\n';
			return -1;
		}
		if (header.column_count >= MAX_COLUMN || header.column_count == 0) {
			std::cerr << header.column_count << " is a bad number of columns\n";
			return -1;
		}
		columns.resize(header.column_count);
		last_press.resize(header.column_count);
		holding_columns.resize(header.column_count);

		for (uint32_t i = 0; i < header.column_count; i++) {
			uint32_t column_index;
			READ_(is, &column_index, sizeof(column_index));
			if (column_index >= header.column_count) {
				std::cerr << "column index " << column_index << " out of range " << header.column_count << '\n';
				return -1;
			}
			uint32_t note_count;
			READ_(is, &note_count, sizeof(note_count));
			columns[column_index].notes.resize(note_count);

			uint32_t last_timestamp = 0;
			Note *n = &columns[column_index].notes[0];
			while (note_count--) {
				READ_(is, &n->timestamp, sizeof(n->timestamp));
				if (n->timestamp < last_timestamp) {
					std::cerr << "unordered timestamps\n";
					return -1;
				}
				READ_(is, &n->hold_duration, sizeof(n->hold_duration));
				n->active = true;
				last_timestamp = n->timestamp + n->hold_duration;
				n++;
			}
		}
	} catch (const std::ios_base::failure &err) {
		std::cerr << err.what() << '\n';
		return err.code().value();
	}
	return 0;
#undef READ_
}

void Chart::draw(int64_t t0, int64_t t1, GLuint vao, GLuint vbo, GLuint ebo) {
	points.clear(); // does not deallocate memory, though
	indices.clear(); // ditto
	int sz = columns.size();
	for (int i = 0; i < sz; i++) {
		columns[i].emit_verts(t0, t1, column_height, note_width, note_height, i, points, indices);
	}
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * points.size(), points.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), 0); // why does GL_FLOAT work but GL_INT doesn't, since I'm giving it ints
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

std::pair<Note *, int64_t> Chart::close_note(int column, uint64_t time, uint64_t threshhold) {
	std::pair<Note *, int64_t> res = columns[column].close_note(time, threshhold);
	Note *note = std::get<0>(res);
	if (note && note->hold_duration > 0)
		columns[column].holding_note = note;
	return res;
}

Note *Chart::unhold(int column) {
	Note *res = columns[column].holding_note;
	columns[column].holding_note = nullptr;
	return res;
}

Note *Chart::first_note() {
	Note *first = nullptr;
	for (Column &col : columns) {
		if (col.notes.size() == 0)
			continue;
		Note *n = &col.notes[0];
		if (!first || first->timestamp > n->timestamp)
			first = n;
	}
	return first;
}

int Chart::drop_before(int64_t time) {
	int dropped = 0;
	for (Column &col : columns) {
		dropped += col.drop_before(time);
	}
	return dropped;
}

int Chart::width() {
	return total_columns() * note_width;
}

int Chart::height() {
	return column_height - note_height;
}

int Chart::total_columns() {
	return columns.size();
}
