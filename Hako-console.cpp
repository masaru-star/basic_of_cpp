#include <bits/stdc++.h>
using namespace std;

constexpr int SIZE = 16;
struct Tile {
    string terrain; // "sea", "plain", "forest", "waste"
    string facility; // "", "house", "farm", "factory"
    int pop = 0;
    bool enhanced = false;
};

struct Action {
    string name; // buildFarm, buildHouse, buildFactory
    int x, y;
};

vector<vector<Tile>> mapGrid;
int money = 2500;
int food = 1000;
int population = 0;
int turnCount = 0;
string islandName = "MyIsland";
vector<Action> actionQueue; // FIFO, 最大 20

// Utility
string terrainChar(const string& t, const Tile& tile) {
    if (!tile.facility.empty()) {
        if (tile.facility == "house") return "H";
        if (tile.facility == "farm") return "F";
        if (tile.facility == "factory") return "X";
    }
    if (t == "sea") return "~";
    if (t == "plain") return ".";
    if (t == "forest") return "T";
    if (t == "waste") return "#";
    return "?";
}

string randTerrain() {
    double r = (double)rand() / RAND_MAX;
    if (r < 0.2) return "sea";
    if (r < 0.5) return "plain";
    if (r < 0.7) return "waste";
    return "forest";
}

void initMap() {
    mapGrid.assign(SIZE, vector<Tile>(SIZE));
    for (int y=0;y<SIZE;y++){
        for(int x=0;x<SIZE;x++){
            if (x<4 || y<4 || x>=SIZE-4 || y>=SIZE-4) mapGrid[y][x].terrain = "sea";
            else mapGrid[y][x].terrain = randTerrain();
            mapGrid[y][x].facility = "";
            mapGrid[y][x].pop = 0;
        }
    }
    // 平地に住宅を2つ配置
    vector<pair<int,int>> plains;
    for(int y=4;y<SIZE-4;y++) for(int x=4;x<SIZE-4;x++) if(mapGrid[y][x].terrain=="plain") plains.emplace_back(x,y);
    random_shuffle(plains.begin(), plains.end());
    for(int i=0;i<min((int)plains.size(),2);i++){
        auto [x,y]=plains[i];
        mapGrid[y][x].facility = "house";
        mapGrid[y][x].pop = 25;
        population += 25;
    }
}

void renderMap() {
    cout << "    ";
    for (int x=0;x<SIZE;x++) cout << setw(2) << x;
    cout << "\n";
    for (int y=0;y<SIZE;y++){
        cout << setw(3) << y << " ";
        for (int x=0;x<SIZE;x++){
            cout << setw(2) << terrainChar(mapGrid[y][x].terrain, mapGrid[y][x]);
        }
        cout << "\n";
    }
}

void status() {
    cout << "--- " << islandName << " ---\n";
    cout << "Turn: " << turnCount << "  Money: " << money << "  Food: " << food << "  Population: " << population << "\n";
    cout << "Action Queue: ("<<actionQueue.size()<<"/20)\n";
    for (size_t i=0;i<actionQueue.size();++i){
        cout<<i+1<<":"<<actionQueue[i].name<<" ("<<actionQueue[i].x<<","<<actionQueue[i].y<<") ";
        if (i<2) cout<<"<Next>";
        cout<<"\n";
    }
}

bool enqueueAction(const Action& a) {
    if (actionQueue.size()>=20) return false;
    actionQueue.push_back(a);
    cout << "計画を追加: "<<a.name<<" ("<<a.x<<","<<a.y<<")\n";
    return true;
}

void cancelAction(int idx) {
    if (idx<1||idx> (int)actionQueue.size()) { cout<<"無効な番号\n"; return; }
    auto a=actionQueue[idx-1];
    actionQueue.erase(actionQueue.begin()+idx-1);
    cout<<"計画を撤回しました: "<<a.name<<"\n";
}

void executeAction(const Action& a) {
    // 簡略: 建設のみ
    if (a.x<0||a.x>=SIZE||a.y<0||a.y>=SIZE) { cout<<"範囲外の座標: ("<<a.x<<","<<a.y<<")\n"; return; }
    Tile &t = mapGrid[a.y][a.x];
    if (a.name=="buildFarm"){
        if (t.terrain=="sea") { cout<<"海には建てられません。\n"; return; }
        const int cost=100;
        if (money<cost){ cout<<"資金不足。\n"; return; }
        if (!t.facility.empty()){ cout<<"既に施設があります。\n"; return; }
        money -= cost; t.facility = "farm"; cout<<"農場を建設しました。\n";
    } else if (a.name=="buildFactory"){
        const int cost=100;
        if (money<cost){ cout<<"資金不足。\n"; return; }
        if (!t.facility.empty()){ cout<<"既に施設があります。\n"; return; }
        money-=cost; t.facility="factory"; cout<<"工場を建設しました。\n";
    } else if (a.name=="buildHouse"){
        const int cost=50;
        if (money<cost){ cout<<"資金不足。\n"; return; }
        if (t.terrain=="sea") { cout<<"海には建てられません。\n"; return; }
        if (!t.facility.empty()){ cout<<"既に施設があります。\n"; return; }
        money-=cost; t.facility="house"; t.pop=20; population+=20; cout<<"住宅を建設しました。人口+20\n";
    }
}

void nextTurn() {
    turnCount++;
    // 毎ターン: 農場があれば食料を増やす, 工場があればお金を少し増やす
    int producedFood = 0; int producedMoney = 0;
    for (int y=0;y<SIZE;y++) for (int x=0;x<SIZE;x++){
        auto &t = mapGrid[y][x];
        if (t.facility=="farm") { producedFood += 20; }
        if (t.facility=="factory") { producedMoney += 30; }
    }
    food += producedFood;
    money += producedMoney;

    // 行動キュー先頭2件を実行（例として）
    int toExecute = min((int)actionQueue.size(), 2);
    for (int i=0;i<toExecute;i++) {
        executeAction(actionQueue[i]);
    }
    // 実行した分をキューから削除
    if (toExecute>0) actionQueue.erase(actionQueue.begin(), actionQueue.begin()+toExecute);

    // ランダムイベント: 小さな増減
    if ((rand()%100) < 5) { money += 100; cout<<"臨時収入を得た。+100G\n"; }

    // 更新表示
    status();
}

void saveGameToFile(const string& filename){
    ofstream ofs(filename);
    if(!ofs){ cout<<"セーブファイルを開けません。\n"; return; }
    // 簡易 JSON ライク出力
    ofs << "{\n";
    ofs << "\"money\":"<<money<<",\n";
    ofs << "\"food\":"<<food<<",\n";
    ofs << "\"population\":"<<population<<",\n";
    ofs << "\"turn\":"<<turnCount<<",\n";
    ofs << "\"islandName\":\""<<islandName<<"\",\n";
    ofs << "\"map\":[\n";
    for (int y=0;y<SIZE;y++){
        ofs << "  [";
        for (int x=0;x<SIZE;x++){
            auto &t=mapGrid[y][x];
            ofs << "{\"terrain\":\""<<t.terrain<<"\",\"facility\":\""<<t.facility<<"\",\"pop\":"<<t.pop<<"}";
            if (x<SIZE-1) ofs<<",";
        }
        ofs << "]" << (y<SIZE-1?",":"") << "\n";
    }
    ofs << "]\n";
    ofs << "}\n";
    ofs.close();
    cout<<"セーブしました: "<<filename<<"\n";
}

void loadGameFromFile(const string& filename){
    ifstream ifs(filename);
    if (!ifs){ cout<<"ロードファイルを開けません。\n"; return; }
    // 非常に簡略化したパース — エラー処理は最小限
    string s; string all;
    while (getline(ifs,s)) all += s;
    // map情報を取り出す（この簡易版では map を完全に置き換える）
    // ここではパースを省略し、新規初期化するだけにする
    cout<<"簡易ロード: 実際のパースは未実装のため初期化に戻します。\n";
    initMap();
    saveGameToFile(filename); // overwrite with basic format
}

int main(){
    srand((unsigned)time(nullptr));
    initMap();
    cout<<"簡易箱庭ゲーム（コンソール版）\n";
    cout<<"コマンド: map / status / select x y / queue add <buildFarm|buildFactory|buildHouse> x y / queue cancel N / next / save filename / load filename / quit\n";
    string cmd;
    while (true) {
        cout << "> ";
        if (!(cin >> cmd)) break;
        if (cmd=="map") { renderMap(); }
        else if (cmd=="status") { status(); }
        else if (cmd=="select") {
            int x,y; cin>>x>>y;
            if (x>=0&&x<SIZE&&y>=0&&y<SIZE) {
                auto &t = mapGrid[y][x];
                cout<<"("<<x<<","<<y<<") 地形="<<t.terrain<<" 施設="<<t.facility<<" 人口="<<t.pop<<"\n";
            } else cout<<"範囲外\n";
        }
        else if (cmd=="queue"){
            string sub; cin>>sub;
            if (sub=="add"){
                string act; int x,y; cin>>act>>x>>y;
                Action a{act,x,y}; if(!enqueueAction(a)) cout<<"キューが満杯です。\n";
            } else if (sub=="cancel"){
                int idx; cin>>idx; cancelAction(idx);
            } else cout<<"queue の使い方: queue add <action> x y / queue cancel N\n";
        }
        else if (cmd=="next") { nextTurn(); }
        else if (cmd=="save") { string fn; cin>>fn; saveGameToFile(fn); }
        else if (cmd=="load") { string fn; cin>>fn; loadGameFromFile(fn); updateStatus:; }
        else if (cmd=="quit") break;
        else cout<<"不明なコマンド\n";
    }
    cout<<"終了\n";
    return 0;
}
