#include "cryptage.h"
#include "skStr.h"
#include <iostream>

int main() {
    std::string input;
    std::cout << "Texte à crypter : ";
    std::getline(std::cin, input);

    std::string skKey = "SuperSecretKey123"; // Tu peux randomiser aussi
    std::string skEncrypted = skCrypt::encrypt(input, skKey);

    std::string finalEncrypted = arescrypter(skEncrypted);

    std::cout << "\n--- Code à coller dans ton projet ---\n";
    std::cout << "std::string texte = SkCrypter::decrypt(aresdecrypter(\"" << finalEncrypted << "\"), \"" << skKey << "\");\n";

    return 0;
}
