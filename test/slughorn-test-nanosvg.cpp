//vimrun! ./slughorn-test-nanosvg

// Verifies that slughorn-nanosvg.hpp correctly decomposes SVG path data into
// slughorn curves, and that the round-trip through Atlas produces correct
// Y-up Shape metrics.
//
// Ground truth (from slughorn-test-cairo.cpp):
// A unit right triangle with right angle at bottom-left should produce:
// Shape(w=1 h=1 bx=0 by=1 ...)

// #define SLUGHORN_NANOSVG_IMPLEMENTATION
#include "slughorn-nanosvg.hpp"

#include <iostream>

// A right triangle authored in SVG (Y-down) space:
// (0,0) = top-left in SVG = bottom-left in Y-up
// (100,0) = top-right in SVG = bottom-right in Y-up
// (0,100) = bot-left in SVG = top-left in Y-up
static const std::string TEST_SVG = R"(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100">
  <path fill="red" d="M 0,0 L 100,0 L 0,100 Z"/>
</svg>
)";

int main(int argc, char** argv) {
	// -------------------------------------------------------------------------
	// Step 1: parse SVG and decompose
	// -------------------------------------------------------------------------
	std::string buf = TEST_SVG;

	NSVGimage* image = nsvgParse(buf.data(), "px", 96.0f);

	if(!image) {
		std::cerr << "ERROR: failed to parse SVG" << std::endl;

		return 1;
	}

	const NSVGshape* shape = image->shapes;

	if(!shape) {
		std::cerr << "ERROR: no shapes in SVG" << std::endl;

		nsvgDelete(image);

		return 1;
	}

	slughorn::Atlas::Curves curves;
	slughorn::Matrix transform;

	slughorn::nanosvg::decomposeShapeLocal(shape, curves, transform, 1.0_cv / 100.0_cv);

	std::cout << "=== decomposeShapeLocal ===" << std::endl;
	std::cout << "Curves: " << curves.size() << std::endl;

	for(size_t i = 0; i < curves.size(); i++) std::cout
		<< "  [" << i << "] " << curves[i] << std::endl
	;

	std::cout << "outTransform: " << transform << std::endl;

	// -------------------------------------------------------------------------
	// Step 2: round-trip through atlas
	// -------------------------------------------------------------------------
	slughorn::Atlas atlas;

	slughorn::Atlas::ShapeInfo info;

	info.autoMetrics = true;

	info.curves = curves;

	atlas.addShape(1u, info);
	atlas.build();

	const slughorn::Atlas::Shape* atlasShape = atlas.getShape(1u);

	std::cout << std::endl << "=== Atlas::Shape ===" << std::endl;

	if(atlasShape) std::cout << *atlasShape << std::endl;

	else std::cout << "ERROR: shape not found" << std::endl;

	std::cout << std::endl << "=== Expected ===" << std::endl;
	std::cout << "Shape(w=1 h=1 bx=0 by=1 ...)" << std::endl;

	nsvgDelete(image);

	return 0;
}
