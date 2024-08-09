# TextToImage Encrypter (TTi encrypter)
## By Anshurup Gupta

This is a Text to image encrypter.

This project is a C-based application that takes a text file containing a normal ASCII message and converts it into a noisy and encrypted PNG image. The encryption is based on the Diffie-Hellman Algorithm (DHA) cryptography. The generated PNG can be decrypted back into the original message using the same algorithm.

## Features

- **Text to Image Conversion**: Convert any text file into a noisy PNG image, making the text unreadable without decryption.
- **Encryption**: Implements Diffie-Hellman Algorithm (DHA) for secure encryption.
- **Decryption**: Decrypt the noisy PNG back into the original text using the corresponding decryption key.

## Prerequisites

- **C Compiler**: Ensure you have GCC or any other C compiler installed.
- **FreeImageLibrary**: Install the FreeImage library for handling PNG files. (present with the project)

## File Structure

- `data/`: Contains the words that are stored in the dictionary.
- `decrypt/`: Keep your png in this folder to decrypt.
- `messages/`: Keep your txt in this folder to encrypt.
- `Dependencies`: Included with the folder.
- `Application`: TTi.exe
- `README.md`: This file.

## Usage

### Converting Text to Noisy Encrypted PNG
- `input.txt`: The input text file containing the ASCII message.
- `output.png`: The generated noisy and encrypted PNG image.

### Decrypting PNG to Text
- `encrypted.png`: The noisy and encrypted PNG file.
- `decrypted_output.txt`: The output text file with the decrypted message.

## How It Works

1. **Text to PNG**:
   - The text from the input file is read and encrypted using the DHA algorithm.
   - Use of a hash map to create linked lists of hashed words based on keys
   - The encrypted text is then encoded into a PNG image with added noise to make it unreadable without the decryption key.

2. **PNG to Text**:
   - The noisy PNG is decoded to retrieve the encrypted data.
   - The data is then decrypted using the corresponding DHA key to produce the original message.

## Contributing

Feel free to submit issues and pull requests. Any contributions, such as new features or bug fixes, are welcome!

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Thanks to the authors of the FreeImage library.
- Special thanks to the open-source community for their invaluable resources.
- IIT Patna
