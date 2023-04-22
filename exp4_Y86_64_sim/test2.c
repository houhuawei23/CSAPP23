// 定义一个结构体，表示预测器
typedef struct {
    int counter; // 计数器
    int threshold; // 阈值
} Predictor;

// 初始化预测器
void init_predictor(Predictor *p, int threshold) {
    p->counter = 0;
    p->threshold = threshold;
}

// 预测分支是否会被执行
int predict(Predictor *p) {
    if (p->counter >= p->threshold) {
        return 1; // 预测分支会被执行
    } else {
        return 0; // 预测分支不会被执行
    }
}

// 更新预测器
void update_predictor(Predictor *p, int outcome) {
    if (outcome) {
        p->counter++; // 分支被执行，计数器加1
    } else {
        p->counter--; // 分支未被执行，计数器减1
    }
}

// 定义一个结构体，表示gshare分支预测器
typedef struct {
    int counter; // 计数器
    int threshold; // 阈值
    unsigned int history; // 历史记录
} GSharePredictor;

// 初始化gshare分支预测器
void init_gshare_predictor(GSharePredictor *p, int threshold) {
    p->counter = 0;
    p->threshold = threshold;
    p->history = 0;
}

// 预测分支是否会被执行
int predict_gshare(GSharePredictor *p, unsigned int address) {
    unsigned int index = (address ^ p->history) % 1024; // 计算索引
    if (p->counter >= p->threshold) {
        return 1; // 预测分支会被执行
    } else {
        return 0; // 预测分支不会被执行
    }
}

// 更新gshare分支预测器
void update_gshare_predictor(GSharePredictor *p, int outcome, unsigned int address) {
    unsigned int index = (address ^ p->history) % 1024; // 计算索引
    if (outcome) {
        p->counter++; // 分支被执行，计数器加1
    } else {
        p->counter--; // 分支未被执行，计数器减1
    }
    p->history = (p->history << 1) | outcome; // 更新历史记录
}


