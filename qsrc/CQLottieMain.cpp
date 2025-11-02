#include <CQLottieMain.h>

#include <QApplication>

int
main(int argc, char **argv)
{
  QApplication app(argc, argv);

  std::string filename;
  bool        debug = false;
  bool        print = false;

  for (auto i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      auto arg = std::string(&argv[i][1]);

      if      (arg == "debug")
        debug = true;
      else if (arg == "print")
        print = true;
      else
        std::cerr << "Unhandled option: " << arg << "\n";
    }
    else
      filename = argv[i];
  }

  auto *lottie = new CQLottie;

  lottie->setDebug(debug);
  lottie->setPrint(print);

  if (filename != "") {
    if (! lottie->load(filename))
      std::cerr << "Failed to load '" << filename << "'\n";
  }

  lottie->show();

  return app.exec();
}
