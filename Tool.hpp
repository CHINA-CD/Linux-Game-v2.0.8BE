#include <string>
void Print(const wchar_t*,short);
class Person {
  public:
    Person(short B,short ID): BV(B),Index(0),Use_Saw(false),Use_Mirror(false),Use_Handcuffs(false),Use_Beer(false),ID(ID) {
      for (int i = 0;i < 8;i++) {
        Items[i] = 0;
      }
    }
    //血量(BV)和道具(Item)还有有没有使用手锯(Use_Saw)、有没有使用放大镜(Use_Mirror)、有没有使用手铐(Use_Handcuffs)、有没有使用啤酒(Use_Beer)方便计算
    short BV,Items[8],Index,ID;
    bool Use_Saw,Use_Mirror,Use_Handcuffs,Use_Beer;
    short Find(short ItemsId) {
      for (int i = 0;i < Index;i++) {
        if (Items[i] == ItemsId)
          return i;
      }
      return -1;
    }
    void clear() {
      for (int i = 0;i < Index;i++)
        Items[i] = 0;
      Index = 0;
      if (Use_Saw)
        Use_Saw = false;
      if (Use_Beer)
        Use_Beer = false;
      if (Use_Handcuffs)
        Use_Handcuffs = false;
      if (Use_Mirror)
        Use_Mirror = false;
    }
    bool operator==(Person& r) {
      return r.ID == this->ID;
    }
};
short StringToShort(std::string);
short CharToShort(char);
class Random {
  public:
	Random(unsigned long Seed = 1): Seed(Seed),MAX(1),MIN(0) {}
	void Set_Random(unsigned int MIN,unsigned int MAX) {
		this->MIN = MIN;
		this->MAX = MAX;
	}
	unsigned long Set_Seed(unsigned long Seed) {
		this->Seed = Seed;
		return this->Seed;
	}
	unsigned int Get_Random() {
	   	this->Next_Seed = (this->Seed * 1103515245 + 85) % 2147483648;
	   	this->Seed = this->Next_Seed;
	    return (this->Seed / 65536) % (this->MAX - this->MIN + 1) + this->MIN;
	}
  private:
	unsigned long Seed,Next_Seed;
	unsigned int MAX,MIN;
};