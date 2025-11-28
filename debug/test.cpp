#include <minlib.hpp>
#include <vbe.hpp>
[[gnu::section(".main"), noreturn, gnu::used]] void main() {
	VBEmode_setup(mode_type::graphics, ~0, ~0, ~0);
    printf(outt::e9, "\033[31m ==========================================\r\nabubebbabbuebba\r\n==========================================\r\n");
    printf(outt::e9, "dsjhvkbvdfshjbklhijbvasdblhjivdaslhbjbvlhasdjklhjbkvadsbhjsdvablhjk");
    for(;;);
}
