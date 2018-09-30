#include <vector>

void steal() {
    // Ports are added to the file manually
    // Don't shuffle them to hide the obvious attempt, most people will probably have sequential ports
    std::vector<int> ports = settings::get_io().get_lines<int>("ports.txt");



}