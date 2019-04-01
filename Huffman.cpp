// Huffman codes compressor by Didier Muñoz Díaz

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <set> 
#include <utility>
#include <map>
#include <string>

using namespace std;

///////////////////////////Auxiliar MyBits class

class MyBits{
private:
	std::vector<char> Data;
	int Size;

public:
	MyBits(void){
		Size=0;
	};

	MyBits(const std::string& str, int sz){
		Size = sz;
		for(int i=0; i<str.size(); ++i){
			Data.push_back(str[i]);
		}
	}

	void append(std::vector<bool>& bitss){
		for(int i=0; i<bitss.size(); ++i){
			if(offSet() == 0){ 
				Data.push_back(0);
			}
			setBit(Size, bitss[i]);
			++Size;
		}
	}

	inline int offSet(void){ // Bit siguiente
		return Size%8;
	}
	inline int byteNum(void){ // Byte para siguiente Bit
		return Size/8;
	}
	inline int& size(void){
		return Size;
	}
	inline int dataBytesNum(void){ // Bytes del vector
		return Data.size();
	}

	inline void setBit(int n, bool value){
		char mask = 1;
		int bytenum = n/8;
		int offset = n%8;
		mask = mask << (7-offset);
		if(value){
			Data[bytenum] = Data[bytenum] | mask;
		}else{
			Data[bytenum] = Data[bytenum] & (~mask);
		}
	}
	inline bool getBit(int n){
		char mask = 1;
		int bytenum = n/8;
		int offset = n%8;
		mask = mask << (7-offset);
		mask = Data[bytenum] & mask;
		if(mask) return true;
		else return false;
	}
	
	char* data(void){
		return Data.data();
	}
	void print(void){
		for(int i=0; i<size(); ++i){
			std::cout << getBit(i);
		}
		std::cout << std::endl;
	}
};

//////////////////////////Huffman Tree Nodes

class HuffNode{
private:
	int Frecuency;
	
public:
	HuffNode(void){}
	virtual ~HuffNode(void){};
	
	int frec(void){return Frecuency;}
	virtual void print(void) = 0;
	virtual void print(bool,int) = 0;
	virtual void getCodes(std::vector<bool>&path, std::map<char, std::vector<bool> >& huffcodes) = 0;
	virtual char getKey(MyBits& bitstream, int& counter) = 0;

protected:
	void setFrecuency(int frec_){Frecuency=frec_;}
};

struct isLessHuff {
    bool operator()(HuffNode* izq, HuffNode* der) const {
        return izq->frec() < der->frec();
    }
};

class HuffNodeAux : public HuffNode{
private:
	HuffNode* Izq, *Der;

public:
	HuffNodeAux(HuffNode* izq_, HuffNode* der_){
		Izq = izq_;
		Der = der_;
		setFrecuency(Izq->frec() + Der->frec());
	}
	~HuffNodeAux(void){
		delete Izq;
		delete Der;
	}

	void getCodes(std::vector<bool>& path, std::map<char, std::vector<bool> >& huffcodes){
		path.push_back(0);
		Izq->getCodes(path, huffcodes);
		path.pop_back();
		path.push_back(1);
		Der->getCodes(path, huffcodes);
		path.pop_back();
	}

	char getKey(MyBits& bitstream, int& counter){
		if(counter >= bitstream.size()){
			std::cerr << "Corrupted file" << std::endl;
			return 0;
		}
		bool auxbool = bitstream.getBit(counter);
		counter = counter+1;
		if(auxbool)
			return Der->getKey(bitstream, counter);
		else
			return Izq->getKey(bitstream, counter);
	}

	void print(void){
		std::stringstream ss;
		ss << frec();
		std::cout << ss.str();
		Izq->print(0,ss.str().size());
		Der->print(1,ss.str().size());
	}
	void print(bool bit_, int depth){
		if(bit_)
			for(int i=0; i<depth; ++i) std::cout << " ";
		std::stringstream ss;
		ss << " -" << bit_ << "- " << frec();
		std::cout << ss.str();
		Izq->print(0, depth+ss.str().size());
		Der->print(1, depth+ss.str().size());
	}
};

class HuffNodeLeaf : public HuffNode{
private:
	char Key;

public:
	HuffNodeLeaf(char key_, int frec_){
		Key = key_;
		setFrecuency(frec_);
	}

	~HuffNodeLeaf(void){}

	char key(void){return Key;}

	void getCodes(std::vector<bool>& path, std::map<char, std::vector<bool> >& huffcodes){
		huffcodes[key()] = path;
	}

	char getKey(MyBits& bitstream, int& counter){
		return key();
	}
	
	void print(void){
		std::cout << frec() << ":" << key() << std::endl;
	}
	void print(bool bit_, int depth){
		if(bit_)
			for(int i=0; i<depth; ++i) std::cout << " ";
		std::cout << " -" << bit_ << "- " << frec() << ":" << key() << std::endl;
	}
};

///////////////////////////Huffman class

class Huffman{
private:
	std::map<char, int> Frecs;
	HuffNode* HuffTreeRoot;
	std::map<char, std::vector<bool> > HuffCodes;

	void calculateFrecuencies(istream& data){
		char auxchar;
		Frecs.clear();
		data.get(auxchar);
		while(data.good()){
			Frecs[auxchar]++;
			data.get(auxchar);
		}
	}
	void generateHuffTree(void){
		std::multiset<HuffNode*, isLessHuff> HuffTree;
		for(std::map<char, int>::iterator it=Frecs.begin(); it != Frecs.end(); ++it){
			HuffNode* newnode = new HuffNodeLeaf(get<0>(*it), get<1>(*it));
			HuffTree.insert(newnode);
		}
		while(HuffTree.size() > 1){
			HuffNode* newnode = new HuffNodeAux(*HuffTree.begin(), *(++HuffTree.begin()));
			HuffTree.insert(newnode);
			HuffTree.erase(HuffTree.begin(), ++ ++ HuffTree.begin());
		}
		delete HuffTreeRoot;
		if(HuffTree.size() == 1) HuffTreeRoot = *HuffTree.begin();
	}
	void generateCodes(void){
		std::vector<bool> auxvec;
		HuffCodes.clear();
		HuffTreeRoot->getCodes(auxvec, HuffCodes);
	}
	void saveEncodeToFile(const std::string& in_name){
		std::ifstream arch_in(in_name);
		MyBits auxbits;
		char auxchar;
		arch_in.get(auxchar);
		while(arch_in.good()){
			try{
				std::vector<bool>& bitss = HuffCodes.at(auxchar);
				auxbits.append(bitss);
			}
			catch(...){}
			arch_in.get(auxchar);
		}
		std::ofstream arch_out(in_name + ".huff");
		arch_out << auxbits.size() << " ";
		for(map<char,int>::iterator it = Frecs.begin(); it != Frecs.end(); ++it){
			arch_out << " " << get<1>(*it);
		}
		arch_out << "\n";
		for(map<char,int>::iterator it = Frecs.begin(); it != Frecs.end(); ++it){
 			arch_out << get<0>(*it);
		}
		arch_out.write(auxbits.data(), auxbits.dataBytesNum());
		std::cout << "Generated file: " << in_name << ".huff" << std::endl;
	}

	void saveDecodeToFile(MyBits& bitss, std::string out_name){
		ofstream arch_out(out_name);
		int counter=0;
		while(counter < bitss.size()){
			arch_out << HuffTreeRoot->getKey(bitss, counter);
		}
	}

public:
	Huffman(void){
		HuffTreeRoot = NULL;
	};

	void encodeData(const std::string& file_name){
		std::ifstream arch(file_name);
		if(arch.good()){
			calculateFrecuencies(arch);
			generateHuffTree();
			generateCodes();
			saveEncodeToFile(file_name);
		} else{
			std::cerr << "Error opening file" << std::endl;
		}
	}
	
	void decodeData(const std::string& file_name){
		std::ifstream arch(file_name);
		if(arch.good()){
		int size;
		std::vector<int> frecuencies;
		arch >> size; // First number is size
		// Then table of frecuencies
		while(arch.get() != '\n'){
			int aux;
			arch >> aux;
			frecuencies.push_back(aux);
		}
		Frecs.clear();
		for(int i=0; i < frecuencies.size(); ++i){
		char auxchar = arch.get();
			Frecs[auxchar] = frecuencies[i];
		}
		// Then generate tree and codes
		generateHuffTree();
		generateCodes();
   	stringstream ss;
   	ss << arch.rdbuf();
		MyBits bitss(ss.str(), size);
		saveDecodeToFile(bitss, file_name + ".dec");
		} else{
			std::cerr << "Error opening file" << std::endl;
		}
	}

	void printFrecuencies(void){
		map<char,int>::iterator it = Frecs.begin();
		std::cout << "\nCharacters frecuency table:\n";
		while(it != Frecs.end()){
			std::cout << get<0>(*it) << " " << get<1>(*it) << "\n";
			++it;
		}
	}

	void printHuffmanTree(void){
		std::cout << "\nTree:\n";
		if(HuffTreeRoot != NULL)
			HuffTreeRoot->print();
	}

	void printHuffmanCodes(void){
		if(HuffTreeRoot != NULL){
			generateCodes();
			std::cout << "\nHufmann Codes:\n";
			for(std::map<char, std::vector<bool> >::iterator it=HuffCodes.begin(); it != HuffCodes.end(); ++it){
				std::cout << get<0>(*it) << " ";
				for(std::vector<bool>::const_iterator it2=get<1>(*it).begin(); it2 != get<1>(*it).end(); ++it2)
					std::cout << *it2;
				std::cout << std::endl;
			}
		}
	}
};

/////////////////////Main

int main(){
	Huffman huff;
	std::cout << "Options:\n1.- Compress file\n2.- Decompress file\n" << std::endl;
	std::cout << "Enter option: ";
	int option=1;
	std::cin >> option;
	if(option == 1){
		std::cout<< "Enter the name of the file to compress: " ;
		std::string file_name;
		std::cin >> file_name;
		huff.encodeData(file_name);
		huff.printFrecuencies();
		huff.printHuffmanTree();
		huff.printHuffmanCodes();
	} else{
		std::cout<< "Enter the name of the file to decompress: " ;
		std::string file_name;
		std::cin >> file_name;
		huff.decodeData(file_name);
		huff.printFrecuencies();
		huff.printHuffmanTree();
		huff.printHuffmanCodes();
	}
return 0;
}
