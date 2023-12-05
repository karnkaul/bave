#include <bave/android_app.hpp>
#include <flappy.hpp>

extern "C" {
void android_main(android_app* andr_app) {
	auto app = bave::AndroidApp{*andr_app};
	app.set_game(std::make_unique<Flappy>(app));
	app.run();
}
}
