#include"SAudioMuxElement.h"
namespace Seraphim{
	std::ofstream& operator<<(std::ofstream&o,SAudioMuxElement& m ){
		o<<"SAudioMuxElement:{";
		SStreamMuxConfig mux = *(m.getStreamMuxConfig());
		//o<<mux;
		o<<"[=";
		o<<mux<<"]";
		o<<"}";
		return o;
	}
};