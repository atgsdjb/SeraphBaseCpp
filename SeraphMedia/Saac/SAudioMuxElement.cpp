#include"SAudioMuxElement.h"
namespace Seraphim{
	std::ostream& operator<<(std::ostream&o,SAudioMuxElement& m ){
		o<<"SAudioMuxElement:{";
		o<<"[="<<m.getStreamMuxConfig()<<"";
		return o;
	}
};