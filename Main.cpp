#include <ncurses.h>
#include <vector>
#include "Tool.hpp"
#include <fstream>
#include <algorithm>
#include <random>
#include <stdlib.h>
WINDOW* mwin = initscr();
const short Menu_Widht = 30,Menu_Height = 7,Introd_Widht = 45,Introd_Height = 8,Exp_Height = 6,Exp_Widht = 27,See_Bullets = 2500,Setting_Height = 3,Setting_Width = 34,OFF_ON_Pos = 22,Pause = 3000;
int Select_Y = 1,Select_X = 2,Bullet_Phase_Index = 0,Bullet_Round_Index = 0,MAX_BV = 0,Bonus = 70000,Use_Beer = 0,Use_Saw = 0,Use_Mirror = 0,Use_Handcuffs = 0,Use_Smoke = 0;
//存储当前回合子弹的vector
std::vector<bool> Bullets;
//存储“你”上一回合有没有使用手铐，主要防止连续使用手铐
bool Up_Use_Handcuffs = false;
//第一个阶段就是第一大组，第二个阶段是第二大组，如果使用了烟导致需要多使用一组的子弹就随机一组，第三阶段是第一大组和第二大组随机
const std::vector<std::vector<std::vector<bool>>> All_Bulltes = {
  {
    {true,false,false},
    {true,true,true,false,false}
  },{
    {true,false},
    {true,true,false,false},
    {true,true,true,false,false},
    {true,true,true,false,false,false},
    {true,true,true,true,true,false,false},
    {true,true,true,false,false,false,false},
    {true,true,false,false,true,false},
    {true,true,false,true,false}
  }
};
//这个回合添加那一组子弹的随机数还有控制这一回合添加什么道具的随机数
Random Round_Bullet(time(nullptr)),R_Add_Item(time(nullptr));
//真子弹和假子弹的数量，用于给机器人判断
short True = 0,False = 0;
class Data {
  public:
    std::fstream RO;
    const std::string Data_Route;
    Data(const std::string &route): Data_Route(route) {
      RO.open(Data_Route,std::ios::in);
      std::string value;
      if (RO.is_open() == false) {
        RO.open(Data_Route,std::ios::out);
        if (RO.is_open() == false) {
          mvprintw(LINES / 2,COLS / 2 - 26 / 2,"致命错误: 无法创建数据文件");
          refresh();
          getchar();
          endwin();
          exit(-1);
        }
      }
      RO.close();
      flush();
      //设置数据默认值
      if (AllData[0] == 0) {
        AllData[0] = 1;
      }
    }
    void flush() {
      std::string value;
      RO.open(Data_Route,std::ios::in);
      for (int i = 0;true;i++) {
        std::getline(RO,value);
        if (!value.empty())
          AllData[i] = StringToShort(value);
        else
          AllData[i] = 0;
        if (RO.eof()) {
          RO.close();
          return;
        }
      }
    }
    /*
      这个写成数组是因为构造的读文件写数据那块好写一点
      1.上次玩到了第几个阶段
    */
    short AllData[2];
} DATA(std::string("/../usr/var/DATA.txt").insert(0,getenv("HOME")));
//这里+1是因为""这个字符数组字面量末尾有\0也占一个字符大小
wchar_t Menu[Menu_Height][Menu_Widht + 1] {
  L"┌────────────────────────────┐",
  L"│   *开始游戏                │",
  L"│   *游戏介绍                │",
  L"│   *操作说明                │",
  L"│   *设置                    │",
  L"│   *退出                    │",
  L"└────────────────────────────┘"
},Introduction[Introd_Height][Introd_Widht + 1] {
  L"┌───────────────────────────────────────────┐",
  L"│ 这是一个模仿俄罗斯轮盘赌的游戏，大部分操  │",
  L"│ 作都使用wasd和换行操作，作者是CD_ QQ是241 │",
  L"│ 6145262，游戏大多数部分都使用ncurses库构  │",
  L"│ 成(别问为什么不用GUI来写，问就是Qt我没电  │",
  L"│ 脑学不了啊QAQ)要想玩得使用Linux终端运行， │",
  L"│ 如果有好的建议或BUG可以告诉我awa          │",
  L"└───────────────────────────────────────────┘"
},Exp[Exp_Height][Exp_Widht + 1] {
  L"┌─────────────────────────┐",
  L"│ WASD       上下左右选择 │",
  L"│ Q      退出并返回主菜单 │",
  L"│ 换行               确认 │",
  L"│ 数字           使用道具 │",
  L"└─────────────────────────┘"
},Setting[Setting_Height][Setting_Width + 1] {
  L"┌────────────────────────────────┐",
  L"│ 开枪闪屏                 [Y/N] │",
  L"└────────────────────────────────┘"
};
class Person DEALER(0,2),YOU(0,1);
const wchar_t Select_Char = L'→';
int main() {
  //声明函数部分
  void Game_Init();
  void Game_Menu();
  void Start_Game();
  void Set(short,short);
  void Center_Print(short,short,short,const wchar_t*,bool);
  void Print_Setting(short);
  
  //初始化部分
  Game_Init();
  
  //代码部分,me是游戏主界面的地方
  me:
  while (true) {
    clear();
    Game_Menu();
    refresh();
    flushinp();
    char Input = getchar();
    if ((Input == 'w' or Input == 'W') and Select_Y > 1) {
      Menu[Select_Y][Select_X] = ' ';
      Select_Y--;
    } else if ((Input == 's' or Input == 'S') and Select_Y < Menu_Height - 2) {
      Menu[Select_Y][Select_X] = ' ';
      Select_Y++;
    } else if (Input == 13) {
      if (Select_Y == 1) {
        Start_Game();
      } else if (Select_Y == 2) {
        clear();
        short Y = LINES / 2 - Introd_Height / 2,X = COLS / 2 - Introd_Widht / 2;
        for (int i = 0;i < Introd_Height;i++,Y++) {
          if (i < Introd_Height - 1) Center_Print(Y,X,Introd_Widht,Introduction[i],false);
          else if (i == Introd_Height - 1) Center_Print(Y,X,Introd_Widht,Introduction[i],true);
        }
      } else if (Select_Y == 3) {
        clear();
        short Y = LINES / 2 - Exp_Height / 2,X = COLS / 2 - Exp_Widht / 2;
        for (int i = 0;i < Exp_Height;i++,Y++) {
          if (i < Exp_Height - 1) Center_Print(Y,X,Exp_Widht,Exp[i],false);
          else if (i == Exp_Height - 1) Center_Print(Y,X,Exp_Widht,Exp[i],true);
        }
      } else if (Select_Y == 4) {
        short tmp = Select_Y;
        Select_Y = 1;
        
        while (true) {
          clear();
          Print_Setting(Select_Y);
          refresh();
          flushinp();
          char Input = getchar();
          if ((Input == 'w' or Input == 'W') and Select_Y > 1)
            Select_Y--;
          else if ((Input == 's' or Input == 'S') and Select_Y < Setting_Height - 2)
            Select_Y++;
          else if (Input == 'q' or Input == 'Q')
            break;
          else if (Input == 13) {
            DATA.AllData[Select_Y] == 1 ? Set(DATA.AllData[0],0) : Set(DATA.AllData[0],1);
            DATA.flush();
          }
        }
        
        Select_Y = tmp;
      } else if (Select_Y == 5) {
        endwin();
        return 0;
      }
    }
  }
  
  endwin();
  return 0;
}
void Add_Bullet() {
  if (!Bullets.empty())
    Bullets.clear();
  if (DATA.AllData[0] <= 2) {
    Round_Bullet.Set_Random(0,All_Bulltes[Bullet_Phase_Index].size() - 1);
    
    Bullets = All_Bulltes[Bullet_Phase_Index][Bullet_Round_Index++];
    True = std::count(Bullets.begin(),Bullets.end(),true);
    False = std::count(Bullets.begin(),Bullets.end(),false);
    
    if (Bullet_Round_Index == All_Bulltes[Bullet_Phase_Index].size())
    Bullet_Round_Index = Round_Bullet.Get_Random();
  } else {
    Bullets = All_Bulltes[Bullet_Phase_Index][Bullet_Round_Index++];
    True = std::count(Bullets.begin(),Bullets.end(),true);
    False = std::count(Bullets.begin(),Bullets.end(),false);
    if (Bullet_Round_Index == All_Bulltes[Bullet_Phase_Index].size()) {
      Round_Bullet.Set_Random(0,All_Bulltes[Bullet_Phase_Index].size() - 1);
      Bullet_Round_Index = Round_Bullet.Get_Random();
    }
  }
}
void shuffle() {
  std::shuffle(Bullets.begin(),Bullets.end(),std::default_random_engine(time(nullptr)));
}
void Add_Item(short s) {
  R_Add_Item.Set_Random(1,5);
  for (int i = 1;i <= s;i++) {
    if (DEALER.Index >= 8)
      break;
    DEALER.Items[DEALER.Index++] = R_Add_Item.Get_Random();
  }
  for (int i = 1;i <= s;i++) {
    if (YOU.Index >= 8)
      break;
    YOU.Items[YOU.Index++] = R_Add_Item.Get_Random();
  }
}
void Erase_Item(Person &r,short index) {
  for (int i = index;i < r.Index - 1;i++)
    r.Items[i] = r.Items[i + 1];
  r.Items[r.Index - 1] = 0;
  r.Index--;
}
void Use_Item(Person& r,short index,bool v = false) {
  switch (r.Items[index]) {
    case 1:
      if (!v and r.BV < MAX_BV)
          r.BV++;
      else
      //这里判断是不是为-5是因为-5代表血量是虚命
        if (r.BV < MAX_BV and r.BV != -5)
          r.BV++;
      Erase_Item(r,index);
      if (r == YOU) {
        Use_Smoke++;
        Bonus -= 20;
      }
      break;
    case 2:
      r.Use_Beer = true;
      Erase_Item(r,index);
      if (r == YOU) {
        Use_Beer += 40;
        Bonus -= 14;
      }
      break;
    case 3:
      r.Use_Saw = true;
      Erase_Item(r,index);
      if (r == YOU) {
        Use_Saw++;
        Bonus -= 160;
      }
      break;
    case 4:
      r.Use_Mirror = true;
      Erase_Item(r,index);
      if (r == YOU) {
        Use_Mirror++;
        Bonus -= 55;
      }
      break;
    case 5:
      r.Use_Handcuffs = true;
      Erase_Item(r,index);
      if (r == YOU) {
        Use_Handcuffs++;
        Bonus -= 2300;
      }
      break;
  }
}
void Game_Menu() {
  short y = LINES / 2 - Menu_Height / 2 ,x = COLS / 2 - Menu_Widht / 2;
  Menu[Select_Y][Select_X] = Select_Char;
  mvprintw(LINES - 1,COLS - 8,"v2.0.8BE");
  for (int i = 0;i < Menu_Height;i++,y++) {
    move(y,x);
    for (int j = 0;j < Menu_Widht;j++) {
      if (Menu[i][j] == Select_Char) attron(COLOR_PAIR(1));
      else if (Menu[i][j] == '*' and Menu[i][Select_X] == Select_Char) {
        if (Menu[i][Select_X] == Select_Char) attroff(COLOR_PAIR(1));
        attron(A_ITALIC | COLOR_PAIR(2));
      } else if (Menu[i][j] == L'│' and j > Select_X) {
        attroff(A_ITALIC | COLOR_PAIR(2));
      }
      printw("%lc",Menu[i][j]);
    }
  }
}
void Game_Init() {
  start_color();
  curs_set(0);
  init_pair(1,COLOR_RED,COLOR_BLACK);
  init_pair(2,COLOR_YELLOW,COLOR_BLACK);
  init_pair(3,COLOR_GREEN,COLOR_BLACK);
  init_pair(4,COLOR_BLUE,COLOR_BLACK);
}
//b是是否显示子弹，i是是否显示道具v是是否有虚命
void Game_Interface(bool b,bool i,bool v) {
  short Y = LINES - 2,X = COLS - 20;
  move(Y,X);
  if (v and DEALER.BV <= -5)
    goto YBV;
  printw("DEALER:\t");
  attron(A_ITALIC | COLOR_PAIR(3));
  for (int i = 1;i <= DEALER.BV;i++) {
    if (i <= 2 and v) {
      attroff(A_ITALIC | COLOR_PAIR(3));
      attron(A_ITALIC | A_REVERSE | COLOR_PAIR(3));
    } else if (i == 3 and v) {
      attroff(A_ITALIC | A_REVERSE | COLOR_PAIR(3));
      attron(A_ITALIC | COLOR_PAIR(3));
    }
    printw("|");
  }
  if (DEALER.BV <= 2)
    attroff(A_ITALIC | A_REVERSE | COLOR_PAIR(3));
  else
    attroff(A_ITALIC | COLOR_PAIR(3));
  YBV:
  move(++Y,X);
  if (v and YOU.BV <= -5)
    goto EBV;
  printw("YOU:\t");
  attron(A_ITALIC | COLOR_PAIR(3));
  for (int i = 1;i <= YOU.BV;i++) {
    if (i <= 2 and v) {
      attroff(A_ITALIC | COLOR_PAIR(3));
      attron(A_ITALIC | A_REVERSE | COLOR_PAIR(3));
    } else if (i == 3 and v) {
      attroff(A_ITALIC | A_REVERSE | COLOR_PAIR(3));
      attron(A_ITALIC | COLOR_PAIR(3));
    }
    printw("|");
  }
  if (YOU.BV <= 2)
    attroff(A_ITALIC | A_REVERSE | COLOR_PAIR(3));
  else
    attroff(A_ITALIC | COLOR_PAIR(3));
  EBV:
  if (YOU.Use_Saw) {
    attron(A_ITALIC | COLOR_PAIR(1));
    mvprintw(0,0,"伤害 * 2");
    attroff(A_ITALIC | COLOR_PAIR(1));
  }
  if (YOU.Use_Handcuffs) {
    attron(A_ITALIC);
    mvprintw(YOU.Use_Saw ? 1 : 0,0,"手铐使用中");
    attroff(A_ITALIC);
  } else if (DEALER.Use_Handcuffs) {
    attron(A_ITALIC);
    mvprintw(0,0,"被拷中...");
    attroff(A_ITALIC);
  }
  if (YOU.BV == -5 or DEALER.BV == -5) {
    attron(A_ITALIC | COLOR_PAIR(1));
    mvprintw(0,COLS - 4,"虚命");
    attroff(A_ITALIC | COLOR_PAIR(1));
  }
  
  if (b) {
    move(LINES - 2,COLS / 2 - (Bullets.size() + Bullets.size() - 1) / 2);
    std::vector<bool> tmp;
    if (Bullets.size() >= 3 and DATA.AllData[0] == 3) {
      tmp = Bullets;
      shuffle();
    }
    for (int i = 0;i < Bullets.size();i++) {
      switch (Bullets[i]) {
        case true:
          attron(COLOR_PAIR(1));
          printw("|");
          attroff(COLOR_PAIR(1));
          break;
        default:
          attron(COLOR_PAIR(4));
          printw("|");
          attroff(COLOR_PAIR(4));
      }
      printw(" ");
    }
    if (Bullets.size() >= 3 and DATA.AllData[0] == 3)
      Bullets = tmp;
    else if (DATA.AllData[0] <= 2)
      mvprintw(LINES - 3,COLS / 2 - 10 / 2,"实: %d空: %d",True,False);
  }
  if (i) {
    wchar_t ItemOf(short);
    int Y = LINES - 2;
    move(Y,0);
    printw("DEALER ITEMS: ");
    for (int i = 0;i < DEALER.Index;i++) {
      printw("%lc ",ItemOf(DEALER.Items[i]));
    }
    move(++Y,0);
    printw("YOU ITEM: ");
    for (int i = 0;i < YOU.Index;i++) {
      printw("%lc ",ItemOf(YOU.Items[i]));
    }
  }
}
wchar_t ItemOf(short id) {
  switch (id) {
    case 1:
      return L'烟';
    case 2:
      return L'酒';
    case 3:
      return L'锯';
    case 4:
      return L'镜';
    case 5:
      return L'拷';
    default:
      return L'E';
  }
}
bool Shoot(bool b,bool i,bool v) {//这里的Use指的是使用锯子，返回的是能不能继续枪的控制权
  void Center_Print(short,short,short,const wchar_t*,bool);
  int Y = LINES / 2 - 5 / 2,Select_Y = 1;
  while (true) {
    clear();
    switch (Select_Y) {
      case 1:
        attron(COLOR_PAIR(1));
        Center_Print(Y,COLS / 2 - 6 / 2,6,L"DEALER",false);
        attroff(COLOR_PAIR(1));
        Center_Print(Y + 3,COLS / 2 - 3 / 2,3,L"YOU",false);
        break;
      case 2:
        Center_Print(Y,COLS / 2 - 6 / 2,6,L"DEALER",false);
        attron(COLOR_PAIR(1));
        Center_Print(Y + 3,COLS / 2 - 3 / 2,3,L"YOU",false);
        attroff(COLOR_PAIR(1));
    }
    refresh();
    flushinp();
    char Input = getchar();
    if ((Input == 'w' or Input == 'W') and Select_Y > 1)
      Select_Y--;
    else if ((Input == 's' or Input == 'S') and Select_Y < 2)
      Select_Y++;
    else if (Input == 13) {
      if (Select_Y == 1) {
        bool tmp = Bullets.back();
        Bullets.pop_back();
        clear();
        Game_Interface(b,i,v);
        refresh();
        napms(Pause);
        if (tmp) {
          YOU.Use_Saw ? DEALER.BV -= 2,YOU.Use_Saw = false : DEALER.BV--;
          (v and DEALER.BV <= 2 and DEALER.BV >= 0 and DEALER.BV > -5) and (DEALER.BV = -5);
          beep();
          if (DATA.AllData[1])
            flash();
          clear();
          Game_Interface(b,i,v);
          refresh();
          Center_Print(LINES - 5,COLS / 2 - 6 / 2,3,L"是真弹",false);
          True--;
        } else {
          if (YOU.Use_Saw)
            YOU.Use_Saw = false;
          Center_Print(LINES - 5,COLS / 2 - 6 / 2,3,L"是假弹",false);
          False--;
        }
        return false;
      } else if (Select_Y == 2) {
        bool tmp = Bullets.back();
        Bullets.pop_back();
        clear();
        Game_Interface(b,i,v);
        refresh();
        napms(Pause);
        if (tmp) {
          YOU.Use_Saw ? YOU.BV -= 2,YOU.Use_Saw = false : YOU.BV--;
          (v and YOU.BV <= 2 and YOU.BV >= 0 and YOU.BV > -5) and (YOU.BV = -5);
          beep();
          if (DATA.AllData[1])
            flash();
          clear();
          Game_Interface(b,i,v);
          refresh();
          Center_Print(LINES - 5,COLS / 2 - 6 / 2,3,L"是真弹",false);
          True--;
          return false;
        } else {
          if (YOU.Use_Saw)
            YOU.Use_Saw = false;
          Center_Print(LINES - 5,COLS / 2 - 6 / 2,3,L"是假弹",false);
          False--;
        }
        return true;
      }
    }
  }
}
//i参数是有没有道具可以用v参数是有没有虚命
void AI(bool i,bool v) {
  void Center_Print(short,short,short,const wchar_t*,bool);
  use:
  bool isTrue = false;
  if (i) {
    while (true) {
      clear();
      Game_Interface(false,i,v);
      refresh();
      if (Bullets.empty())
        return;
      if (DEALER.Find(5) != -1 and !DEALER.Use_Handcuffs and True != 0 and Bullets.size() >= 2) {
        mvprintw(LINES - 5,COLS / 2 - 8 / 2,"AI使用了%lc",ItemOf(5));
        refresh();
        napms(Pause);
        Use_Item(DEALER,DEALER.Find(5),v);
      } else if (DEALER.Find(1) != -1 and DEALER.BV < MAX_BV) {
        mvprintw(LINES - 5,COLS / 2 - 8 / 2,"AI使用了%lc",ItemOf(1));
        refresh();
        napms(Pause);
        Use_Item(DEALER,DEALER.Find(1),v);
      } else if (DEALER.Find(4) != -1 and True != 0 and False != 0 and !DEALER.Use_Mirror) {
        mvprintw(LINES - 5,COLS / 2 - 8 / 2,"AI使用了%lc",ItemOf(4));
        refresh();
        napms(Pause);
        Use_Item(DEALER,DEALER.Find(4),v);
        isTrue = Bullets.back();
      } else if ((DEALER.Find(3) != -1 and isTrue and !DEALER.Use_Saw) or (DEALER.Find(3) != -1 and ((True > (Bullets.size() % 2 == 0 ? Bullets.size() / 2 : Bullets.size() / 2 + 1) and !DEALER.Use_Saw) or False == 0 and !DEALER.Use_Saw)) and (DEALER.Use_Mirror ? isTrue : true)) {
        mvprintw(LINES - 5,COLS / 2 - 8 / 2,"AI使用了%lc",ItemOf(3));
        refresh();
        napms(Pause);
        Use_Item(DEALER,DEALER.Find(3),v);
      } else if (DEALER.Find(2) != -1 and True == False and !DEALER.Use_Mirror and ((Bullets.size() == 2 and !DEALER.Use_Handcuffs) or Bullets.size() > 2) and !DEALER.Use_Saw) {
        Use_Item(DEALER,DEALER.Find(2),v);
        mvprintw(LINES - 5,COLS / 2 - 8 / 2,"AI使用了%lc",ItemOf(2));
        refresh();
        napms(Pause);
        bool tmp = Bullets.back();
        Bullets.pop_back();
        clear();
        Game_Interface(false,i,v);
        if (tmp) {
          mvprintw(LINES - 5,COLS / 2 - 10 / 2,"弹出了真弹");
          True--;
        } else {
          mvprintw(LINES - 5,COLS / 2 - 10 / 2,"弹出了假弹");
          False--;
        }
        refresh();
        DEALER.Use_Beer = false;
        napms(Pause);
      } else {
        break;
      }
    }
  }
  
  st:
  clear();
  Game_Interface(false,i,v);
  refresh();
  if (Bullets.empty())
    return;
  bool tmp = Bullets.back();
  Bullets.pop_back();
  if (!DEALER.Use_Mirror) {
    if (True != 0 and (Bullets.size() % 2 == 0 ? True >= Bullets.size() / 2 : True > Bullets.size() / 2)) {
      Center_Print(LINES - 5,COLS / 2 - 14 / 2,9,L"(AI对你开枪了)",false);
      refresh();
      napms(Pause);
      if (tmp) {
        DEALER.Use_Saw ? YOU.BV -= 2,DEALER.Use_Saw = false : YOU.BV--;
        (v and YOU.BV <= 2 and YOU.BV >= 0 and YOU.BV > -5) and (YOU.BV = -5);
        beep();
        if (DATA.AllData[1])
          flash();
        clear();
        Game_Interface(false,i,v);
        Center_Print(LINES - 5,COLS / 2 - 6 / 2,3,L"是真弹",false);
        True--;
        return;
      } else {
        if (DEALER.Use_Saw)
          DEALER.Use_Saw = false;
        clear();
        Game_Interface(false,i,v);
        Center_Print(LINES - 5,COLS / 2 - 6 / 2,3,L"是假弹",false);
        False--;
        return;
      }
    } else {
      Center_Print(LINES - 5,COLS / 2 - 16 / 2,10,L"(AI对自己开枪了)",false);
      refresh();
      napms(Pause);
      if (tmp) {
        DEALER.Use_Saw ? DEALER.BV -= 2,DEALER.Use_Saw = false : DEALER.BV--;
        (v and DEALER.BV <= 2 and DEALER.BV >= 0 and DEALER.BV > -5) and (DEALER.BV = -5);
        beep();
        if (DATA.AllData[1])
          flash();
        clear();
        Game_Interface(false,i,v);
        Center_Print(LINES - 5,COLS / 2 - 6 / 2,3,L"是真弹",false);
        True--;
        return;
      } else {
        if (DEALER.Use_Saw)
          DEALER.Use_Saw = false;
        clear();
        Game_Interface(false,i,v);
        Center_Print(LINES - 5,COLS / 2 - 6 / 2,3,L"是假弹",false);
        napms(Pause);
        False--;
        if (!i)
          goto st;
        else
          goto use;
      }
    }
  } else {
    if (isTrue) {
      Center_Print(LINES - 5,COLS / 2 - 14 / 2,9,L"(AI对你开枪了)",false);
      refresh();
      napms(Pause);
      DEALER.Use_Saw ? YOU.BV -= 2,DEALER.Use_Saw = false : YOU.BV--;
      (v and YOU.BV <= 2 and YOU.BV >= 0 and YOU.BV > -5) and (YOU.BV = -5);
      beep();
      if (DATA.AllData[1])
        flash();
      clear();
      Game_Interface(false,i,v);
      Center_Print(LINES - 5,COLS / 2 - 6 / 2,3,L"是真弹",false);
      True--;
      DEALER.Use_Mirror = false;
    } else {
      Center_Print(LINES - 5,COLS / 2 - 16 / 2,10,L"(AI对自己开枪了)",false);
      refresh();
      napms(Pause);
      if (DEALER.Use_Saw)
        DEALER.Use_Saw = false;
      clear();
      Game_Interface(false,i,v);
      Center_Print(LINES - 5,COLS / 2 - 6 / 2,3,L"是假弹",false);
      napms(Pause);
      False--;
      DEALER.Use_Mirror = false;
      goto use;
    }
  }
}
void Set(short Phase,short Fflush = DATA.AllData[1]) {
  DATA.RO.open(DATA.Data_Route,std::ios::out);
  
  DATA.RO << Phase << std::endl;
  DATA.RO << Fflush << std::endl;
  
  DATA.RO.close();
}
void Print_Setting(short y) {
  short Y = LINES / 2 - Setting_Height / 2,X = COLS / 2 - Setting_Width / 2;
  for (int i = 0;i < Setting_Height;i++) {
    move(Y++,X);
    for (int j = 0;j < Setting_Width;j++) {
      if (i == y and j == 1)
        attron(COLOR_PAIR(2));
      if (Setting[i][j] == L'[' and i == y)
        attroff(COLOR_PAIR(2));
      else if (Setting[i][j] == L'Y' and DATA.AllData[y] == 1) {
        attron(COLOR_PAIR(3));
        printw("%lc",Setting[i][j]);
        attroff(COLOR_PAIR(3));
        continue;
      }
      else if (Setting[i][j] == L'Y' and DATA.AllData[y] == 0) {
        attron(COLOR_PAIR(1));
        printw("%lc",Setting[i][j]);
        attroff(COLOR_PAIR(1));
        continue;
      } else if (Setting[i][j] == L'N' and DATA.AllData[y] == 0) {
        attron(COLOR_PAIR(3));
        printw("%lc",Setting[i][j]);
        attroff(COLOR_PAIR(3));
        continue;
      }
      else if (Setting[i][j] == L'N' and DATA.AllData[y] == 1) {
        attron(COLOR_PAIR(1));
        printw("%lc",Setting[i][j]);
        attroff(COLOR_PAIR(1));
        continue;
      }
      printw("%lc",Setting[i][j]);
    }
  }
}
short Phase1() {
  Set(1);
  DATA.flush();
  
  add_bullet:
  Add_Bullet();
  clear();
  Game_Interface(true,false,false);
  refresh();
  napms(See_Bullets);
  shuffle();
  
  while (true) {
    clear();
    Game_Interface(false,false,false);
    refresh();
    if (YOU.BV <= 0)
      return 2;
    else if (DEALER.BV <= 0)
      return 3;
    else if (Bullets.empty())
      goto add_bullet;
    
    flushinp();
    char Input = getchar();
    if (Input == 'q' or Input == 'Q')
      return 1;
    bool tmp = Shoot(false,false,false);
    napms(Pause);
    if (!tmp) {
      if (YOU.BV <= 0)
        return 2;
      else if (DEALER.BV <= 0)
        return 3;
    AI(false,false);
    napms(Pause);
   }
  }
}
short Phase23(bool v = false) {
  add_bullet:
  YOU.Use_Handcuffs = false,Up_Use_Handcuffs = false,DEALER.Use_Handcuffs = false;
  Add_Bullet();
  if (DATA.AllData[0] == 2)
    Add_Item(2);
  else if (DATA.AllData[0] == 3)
    Add_Item(4);
  clear();
  Game_Interface(true,true,v);
  refresh();
  napms(See_Bullets);
  shuffle();
  
  while (true) {
    clear();
    Game_Interface(false,true,v);
    refresh();
    if (v ? YOU.BV <= 0 and YOU.BV != -5 : YOU.BV <= 0)
      return 2;
    else if (v ? DEALER.BV <= 0 and DEALER.BV != -5 : DEALER.BV <= 0)
      return 3;
    else if (Bullets.empty())
      goto add_bullet;
    
    if (!DEALER.Use_Handcuffs) {
      flushinp();
      char Input = getchar();
      if (Input == 'q' or Input == 'Q')
        return 1;
      else if (Input >= '1' and Input <= '8') {
        while (true) {
          if (Input >= '1' and Input <= '8' and CharToShort(Input) - 1 < YOU.Index) {
            if (YOU.Use_Saw and YOU.Items[CharToShort(Input) - 1] == 3) {
              flushinp();
              Input = getchar();
              continue;
            } else if (YOU.Use_Handcuffs and YOU.Items[CharToShort(Input) - 1] == 5) {
              flushinp();
              Input = getchar();
              continue;
            } else if (YOU.Items[CharToShort(Input) - 1] == 5)
              Up_Use_Handcuffs = true;
            mvprintw(LINES - 5,COLS / 2 - 13 / 2,"你使用了%lc...",ItemOf(YOU.Items[CharToShort(Input) - 1]));
            refresh();
            napms(Pause);
            Use_Item(YOU,CharToShort(Input) - 1,v);
            if (YOU.Use_Mirror) {
              clear();
              Game_Interface(false,true,v);
              refresh();
              Bullets.back() ? mvprintw(LINES - 5,COLS / 2 - 6 / 2,"是真弹") : mvprintw(LINES - 5,COLS / 2 - 6 / 2,"是假弹");
              YOU.Use_Mirror = false;
              refresh();
              napms(Pause);
            } else if (YOU.Use_Beer) {
              bool tmp = Bullets.back();
              Bullets.pop_back();
              clear();
              Game_Interface(false,true,v);
              refresh();
              tmp ? mvprintw(LINES - 5,COLS / 2 - 10 / 2,"弹出了真弹"),True-- : (mvprintw(LINES - 5,COLS / 2 - 10 / 2,"弹出了假弹"),False--);
              YOU.Use_Beer = false;
              refresh();
              napms(Pause);
              if (Bullets.empty())
                goto add_bullet;
            }
            clear();
            Game_Interface(false,true,v);
            refresh();
            flushinp();
            Input = getchar();
          } else if (Input >= '1' and Input <= '8' and CharToShort(Input) - 1 >= YOU.Index) {
            clear();
            Game_Interface(false,true,v);
            refresh();
            flushinp();
            Input = getchar();
          } else if (Input == 'q' or Input == 'Q') {
            return 1;
          } else {
            break;
          }
        }
      }
    }
    if (!DEALER.Use_Handcuffs) {
      bool tmp = Shoot(false,true,v);
      napms(Pause);
      if (v ? YOU.BV <= 0 and YOU.BV != -5 : YOU.BV <= 0)
        return 2;
      else if (v ? DEALER.BV <= 0 and DEALER.BV != -5 : DEALER.BV <= 0)
        return 3;
      else if (Bullets.empty())
        goto add_bullet;
      if (!tmp) {
        Up_Use_Handcuffs ? Up_Use_Handcuffs = false : YOU.Use_Handcuffs and (YOU.Use_Handcuffs = false);
        if (!YOU.Use_Handcuffs) {
          AI(true,v);
          napms(Pause);
        }
      }
    } else {
      AI(true,v);
      DEALER.Use_Handcuffs = false;
      napms(Pause);
    }
  }
}
void Animations_Print(short y,short x,const wchar_t* ptr,short length,short ms) {
  move(y,x);
  for (int i = 0;i < length;i++) {
    printw("%lc",ptr[i]);
    refresh();
    napms(ms);
  }
}
void Start_Game() {
  void Center_Print(short,short,short,const wchar_t*,bool);
  
  const short AnimationsMs = 100;
  beep();
  for (int i = 1;i <= 8;i++) {
    napms(AnimationsMs);
    init_pair(2,COLOR_WHITE,COLOR_BLACK);
    clear();
    Game_Menu();
    refresh();
    init_pair(2,COLOR_YELLOW,COLOR_BLACK);
    
    napms(AnimationsMs);
    clear();
    Game_Menu();
    refresh();
  }
  napms(1000);
  
  if (DATA.AllData[0] == 2)
    goto ph2;
  else if (DATA.AllData[0] == 3)
    goto ph3;
  
  MAX_BV = 2;
  while (true) {
    DEALER.BV = MAX_BV;
    YOU.BV = MAX_BV;
    True = 0;
    False = 0;
    Bullet_Phase_Index = 0;
    Bullet_Round_Index = 0;
    short end = Phase1();
    if (end == 1)
      return;
    else if (end == 2) {
      clear();
      mvprintw(LINES / 2,COLS / 2 - 29 / 2,"你得庆幸它还给你留了点电量...");
      refresh();
      napms(Pause);
      continue;
    } else if (end == 3) {
      break;
    }
  }
  
  clear();
  mvprintw(LINES / 2,COLS / 2 - 29 / 2,"让我们把这个游戏变得有趣点...");
  refresh();
  napms(Pause);
  
  ph2:
  MAX_BV = 4;
  Set(2);
  DATA.flush();
  while (true) {
    DEALER.BV = MAX_BV;
    YOU.BV = MAX_BV;
    True = 0;
    False = 0;
    Bullet_Phase_Index = 1;
    Bullet_Round_Index = 0;
    YOU.clear();
    DEALER.clear();
    short end = Phase23();
    if (end == 1)
      return;
    else if (end == 2) {
      clear();
      mvprintw(LINES / 2,COLS / 2 - 29 / 2,"你得庆幸它还给你留了点电量...");
      refresh();
      napms(Pause);
      continue;
    } else if (end == 3) {
      break;
    }
  }
  
  clear();
  mvprintw(LINES / 2,COLS / 2 - 34 / 2,"(不知道说什么，反要到第三个阶段了)");
  refresh();
  napms(Pause);
  
  ph3:
  MAX_BV = 6;
  Set(3);
  DATA.flush();
  DEALER.BV = MAX_BV;
  YOU.BV = MAX_BV;
  DEALER.clear();
  YOU.clear();
  True = 0;
  False = 0;
  Bullet_Phase_Index = 1;
  Bullet_Round_Index = 0;
  short end = Phase23(true);
  if (end == 1)
    return;
  else if (end == 2) {
    clear();
    mvprintw(LINES / 2,COLS / 2 - 18 / 2,"很遗憾，你失败了:(");
    refresh();
    napms(Pause);
    Set(1);
    DATA.flush();
    return;
  }
  
  clear();
  int y = LINES / 2 - 7 / 2;
  Animations_Print(y++,COLS / 2 - 13 / 2,L"你赢了! 恭喜!",8,AnimationsMs);
  mvprintw(y++,COLS / 2 - 16 / 2,"共使用放大镜%02d次",Use_Mirror);
  beep();
  refresh();
  napms(AnimationsMs + 100);
  mvprintw(y++,COLS / 2 - 14 / 2,"共使用手锯%02d次",Use_Saw);
  beep();
  refresh();
  napms(AnimationsMs + 100);
  mvprintw(y++,COLS / 2 - 16 / 2,"共消耗酒精%02d毫升",Use_Beer);
  beep();
  refresh();
  napms(AnimationsMs + 100);
  mvprintw(y++,COLS / 2 - 14 / 2,"共使用手铐%02d个",Use_Handcuffs);
  beep();
  refresh();
  napms(AnimationsMs + 100);
  mvprintw(y++,COLS / 2 - 14 / 2,"共使用香烟%02d支",Use_Smoke);
  beep();
  refresh();
  napms(AnimationsMs);
  mvprintw(y,COLS / 2 - 15 / 2,"总获得金额%d",Bonus);
  beep();
  mvprintw(LINES - 1,COLS / 2 - 14 / 2,"(按任意键返回)");
  refresh();
  flushinp();
  getchar();
  Bonus = 70000;
  Use_Handcuffs = 0;
  Use_Beer = 0;
  Use_Mirror = 0;
  Use_Smoke = 0;
  Use_Saw = 0;
  DEALER.clear();
  YOU.clear();
  Set(1);
  DATA.flush();
}
void Center_Print(short Y,short X,short len,const wchar_t *ptr,bool r) {
    move(Y,X);
    Print(ptr,len);
    if (r) {
      X = COLS / 2 - 14 / 2;
      move(++Y,X);
      Print(L"(按任意键返回)",8);
      flushinp();
      getchar();
    }
    refresh();
}