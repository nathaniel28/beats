#ifndef CHART_HPP
#define CHART_HPP

#include <cstdint>
#include <istream>
#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>

#define MAX_COLUMN 8

// a point given a pixel coordinate on screen
// we do rely on the fact that a point is 64 bits, and a vector<Point> is packed
// (though I can't imagine a vector<Point> with each point consuming more or
// less than 64 bits)
struct Point {
	int32_t x;
	int32_t y;
};

struct Note {
	uint32_t timestamp; // in milliseconds
	uint32_t hold_duration; // 0 for a press
	bool active; // whether or not to consider the note needing to be pressed
};

class Column {
public:
	std::vector<Note> notes;
	Note *holding_note;
private:
	int note_index, drop_index;
public:
	Column();

	void emit_verts(int64_t t0, int64_t t1, int column_height, int note_width, int note_height, int column_offset, std::vector<Point> &points, std::vector<uint32_t> &indices);

	std::pair<Note *, int64_t> close_note(uint64_t time, uint64_t threshhold);

	int drop_before(int64_t time);

	//void reset();
};

class Chart {
	std::vector<Column> columns;
	std::vector<Point> points; // points of rectangles to draw notes
	std::vector<uint32_t> indices; // indices for the EBO to help draw points
public:
	std::vector<uint32_t> last_press; // last press of a column, millisecond offset from song start, u32 for GLSL
	std::vector<uint8_t> holding_columns; // columns held last frame; uint8_t instead of bool to avoid weird feature
private:
	int column_height;
	int note_width;
	int note_height;

	// file signature
	static const uint64_t magic = 0xF1E0007472616863;
	static const uint32_t version = 2;

public:
	// calling the constructor does not make a Chart ready to use. (sorry)
	// you must call deserialize next
	Chart(uint32_t col_height, uint32_t note_width_, uint32_t note_height_);

	// on success this function returns 0 and this Chart is safe to use
	// calls istream::exceptions
	int deserialize(std::istream &);

	//int serialize(std::ostream &);

	// draw the notes from t0 to t1, also updates note_index
	// future calls should be made with a greater t0 and t1
	void draw(int64_t t0, int64_t t1, GLuint vao, GLuint vbo, GLuint ebo);

	// returns the closest *unpressed* note and the absolute time difference
	// between time and that note's timestamp, where column is the offset of
	// the column to look for notes in, and threshhold is how far in the
	// past/future to search
	std::pair<Note *, int64_t> close_note(int column, uint64_t time, uint64_t threshhold);

	Note *unhold(int column);

	Note *first_note();

	// similar to draw calls,
	// future calls should be made with a greater time
	int drop_before(int64_t time);

	int width();

	int height();

	int total_columns();
};

#endif
