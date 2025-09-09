/**
 * @file nlp_processor.c
 * @brief NLP Processor Implementation for ESP32-S3 AI Assistant
 * @author AI Assistant Team
 * @version 1.0.0
 */

#include "nlp_processor.h"
#include "ai_assistant.h"
#include "esp_log.h"
#include <string.h>
#include <ctype.h>

static const char *TAG = "NLP_PROCESSOR";

// NLP处理器状态和配置
static nlp_processor_state_t s_nlp_state = NLP_PROCESSOR_STATE_IDLE;
static nlp_processor_config_t s_nlp_config = {
    .language = 0, // 0=中文, 1=英文
    .enable_entity_recognition = true,
    .enable_sentiment_analysis = true,
    .enable_keyword_extraction = true,
    .confidence_threshold = 0.8f,
    .model_path = "/spiffs/nlp_model.bin"
};

// 关键词映射表
typedef struct {
    const char *keywords[10];
    voice_command_type_t command_type;
} keyword_mapping_t;

// 中文关键词映射
static const keyword_mapping_t chinese_keywords[] = {
    {{"天气", "气温", "温度", "下雨", "晴天", "阴天", "雪", "风", NULL}, VOICE_CMD_WEATHER},
    {{"时间", "几点", "现在", "日期", "星期", "今天", "明天", "昨天", NULL}, VOICE_CMD_TIME},
    {{"新闻", "消息", "资讯", "头条", "报道", "事件", NULL}, VOICE_CMD_NEWS},
    {{"音乐", "歌曲", "播放", "暂停", "下一首", "上一首", "音量", NULL}, VOICE_CMD_MUSIC},
    {{"开灯", "关灯", "灯光", "亮度", "照明", "台灯", "吊灯", NULL}, VOICE_CMD_LIGHT},
    {{"温度", "空调", "暖气", "制冷", "制热", "风扇", NULL}, VOICE_CMD_TEMPERATURE},
    {{"设置", "配置", "参数", "选项", "偏好", "设定", NULL}, VOICE_CMD_SETTINGS},
    {{"帮助", "帮忙", "怎么", "如何", "教程", "说明", NULL}, VOICE_CMD_HELP}
};

// 英文关键词映射
static const keyword_mapping_t english_keywords[] = {
    {{"weather", "temperature", "rain", "sunny", "cloudy", "snow", "wind", NULL}, VOICE_CMD_WEATHER},
    {{"time", "clock", "date", "today", "tomorrow", "yesterday", "now", NULL}, VOICE_CMD_TIME},
    {{"news", "headlines", "report", "story", "update", NULL}, VOICE_CMD_NEWS},
    {{"music", "song", "play", "pause", "next", "previous", "volume", NULL}, VOICE_CMD_MUSIC},
    {{"light", "lamp", "brightness", "on", "off", "illuminate", NULL}, VOICE_CMD_LIGHT},
    {{"temperature", "air", "heating", "cooling", "fan", "warm", "cold", NULL}, VOICE_CMD_TEMPERATURE},
    {{"settings", "config", "options", "preferences", "setup", NULL}, VOICE_CMD_SETTINGS},
    {{"help", "how", "tutorial", "guide", "instruction", NULL}, VOICE_CMD_HELP}
};

// 停用词列表
static const char *chinese_stopwords[] = {
    "的", "了", "在", "是", "我", "有", "和", "就", "不", "人", "都", "一", "一个", "上", "也", "很", "到", "说", "要", "去", "你", "会", "着", "没有", "看", "好", "自己", "这", NULL
};

static const char *english_stopwords[] = {
    "the", "a", "an", "and", "or", "but", "in", "on", "at", "to", "for", "of", "with", "by", "is", "are", "was", "were", "be", "been", "have", "has", "had", "do", "does", "did", "will", "would", "could", "should", NULL
};

/**
 * @brief 初始化NLP处理器
 */
esp_err_t nlp_processor_init(void)
{
    ai_assistant_log_info(TAG, "Initializing NLP Processor");
    
    // 加载NLP模型（如果存在）
    if (strlen(s_nlp_config.model_path) > 0) {
        esp_err_t ret = nlp_processor_load_model(s_nlp_config.model_path);
        if (ret != ESP_OK) {
            ai_assistant_log_info(TAG, "NLP model not found, using built-in rules");
        }
    }
    
    s_nlp_state = NLP_PROCESSOR_STATE_IDLE;
    ai_assistant_log_info(TAG, "NLP Processor initialized successfully");
    return ESP_OK;
}

/**
 * @brief 设置NLP处理器配置
 */
esp_err_t nlp_processor_set_config(const nlp_processor_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Config pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&s_nlp_config, config, sizeof(nlp_processor_config_t));
    ai_assistant_log_info(TAG, "NLP processor configuration updated");
    return ESP_OK;
}

/**
 * @brief 获取NLP处理器配置
 */
esp_err_t nlp_processor_get_config(nlp_processor_config_t *config)
{
    if (config == NULL) {
        ai_assistant_log_error(TAG, "Config pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(config, &s_nlp_config, sizeof(nlp_processor_config_t));
    return ESP_OK;
}

/**
 * @brief 分类语音命令
 */
voice_command_type_t nlp_processor_classify_command(const char *text)
{
    if (text == NULL || strlen(text) == 0) {
        ai_assistant_log_error(TAG, "Text cannot be NULL or empty");
        return VOICE_CMD_UNKNOWN;
    }
    
    ai_assistant_log_info(TAG, "Classifying command: %s", text);
    
    s_nlp_state = NLP_PROCESSOR_STATE_PROCESSING;
    
    // 预处理文本
    char processed_text[NLP_MAX_TEXT_LEN];
    nlp_processor_preprocess_text(text, processed_text, sizeof(processed_text));
    
    // 选择关键词映射表
    const keyword_mapping_t *keywords;
    size_t keyword_count;
    
    if (s_nlp_config.language == 0) { // 中文
        keywords = chinese_keywords;
        keyword_count = sizeof(chinese_keywords) / sizeof(chinese_keywords[0]);
    } else { // 英文
        keywords = english_keywords;
        keyword_count = sizeof(english_keywords) / sizeof(english_keywords[0]);
    }
    
    // 匹配关键词
    voice_command_type_t best_match = VOICE_CMD_UNKNOWN;
    int max_matches = 0;
    
    for (size_t i = 0; i < keyword_count; i++) {
        int matches = 0;
        for (int j = 0; keywords[i].keywords[j] != NULL; j++) {
            if (strstr(processed_text, keywords[i].keywords[j]) != NULL) {
                matches++;
            }
        }
        
        if (matches > max_matches) {
            max_matches = matches;
            best_match = keywords[i].command_type;
        }
    }
    
    s_nlp_state = NLP_PROCESSOR_STATE_IDLE;
    
    ai_assistant_log_info(TAG, "Command classified as: %s", voice_command_get_type_string(best_match));
    return best_match;
}

/**
 * @brief 提取意图
 */
esp_err_t nlp_processor_extract_intent(const char *text, char *intent, size_t intent_len)
{
    if (text == NULL || intent == NULL || intent_len == 0) {
        ai_assistant_log_error(TAG, "Invalid parameters");
        return ESP_ERR_INVALID_ARG;
    }
    
    ai_assistant_log_info(TAG, "Extracting intent from: %s", text);
    
    s_nlp_state = NLP_PROCESSOR_STATE_PROCESSING;
    
    // 分类命令类型
    voice_command_type_t cmd_type = nlp_processor_classify_command(text);
    
    // 将命令类型转换为意图字符串
    const char *intent_str = voice_command_get_type_string(cmd_type);
    strncpy(intent, intent_str, intent_len - 1);
    intent[intent_len - 1] = '\0';
    
    s_nlp_state = NLP_PROCESSOR_STATE_IDLE;
    
    ai_assistant_log_info(TAG, "Extracted intent: %s", intent);
    return ESP_OK;
}

/**
 * @brief 提取实体
 */
esp_err_t nlp_processor_extract_entities(const char *text, nlp_entity_t *entities, uint8_t *entity_count)
{
    if (text == NULL || entities == NULL || entity_count == NULL) {
        ai_assistant_log_error(TAG, "Invalid parameters");
        return ESP_ERR_INVALID_ARG;
    }
    
    ai_assistant_log_info(TAG, "Extracting entities from: %s", text);
    
    s_nlp_state = NLP_PROCESSOR_STATE_PROCESSING;
    
    *entity_count = 0;
    
    // 简单的实体识别 - 基于规则
    char *text_copy = strdup(text);
    if (text_copy == NULL) {
        s_nlp_state = NLP_PROCESSOR_STATE_IDLE;
        return ESP_ERR_NO_MEM;
    }
    
    // 转换为小写
    for (char *p = text_copy; *p; p++) {
        *p = tolower(*p);
    }
    
    // 查找数字实体
    for (char *p = text_copy; *p && *entity_count < NLP_MAX_ENTITIES; p++) {
        if (isdigit(*p)) {
            char *start = p;
            while (*p && (isdigit(*p) || *p == '.')) p++;
            
            size_t len = p - start;
            if (len > 0 && len < NLP_MAX_ENTITY_LEN - 1) {
                strncpy(entities[*entity_count].text, start, len);
                entities[*entity_count].text[len] = '\0';
                entities[*entity_count].type = NLP_ENTITY_TYPE_NUMBER;
                entities[*entity_count].confidence = 0.9f;
                entities[*entity_count].start_pos = start - text_copy;
                entities[*entity_count].end_pos = p - text_copy;
                (*entity_count)++;
            }
            p--;
        }
    }
    
    // 查找时间实体
    const char *time_patterns[] = {"今天", "明天", "昨天", "现在", "早上", "中午", "下午", "晚上", "夜里", NULL};
    for (int i = 0; time_patterns[i] != NULL && *entity_count < NLP_MAX_ENTITIES; i++) {
        char *pos = strstr(text_copy, time_patterns[i]);
        if (pos != NULL) {
            strncpy(entities[*entity_count].text, time_patterns[i], NLP_MAX_ENTITY_LEN - 1);
            entities[*entity_count].text[NLP_MAX_ENTITY_LEN - 1] = '\0';
            entities[*entity_count].type = NLP_ENTITY_TYPE_TIME;
            entities[*entity_count].confidence = 0.8f;
            entities[*entity_count].start_pos = pos - text_copy;
            entities[*entity_count].end_pos = pos - text_copy + strlen(time_patterns[i]);
            (*entity_count)++;
        }
    }
    
    free(text_copy);
    s_nlp_state = NLP_PROCESSOR_STATE_IDLE;
    
    ai_assistant_log_info(TAG, "Extracted %d entities", *entity_count);
    return ESP_OK;
}

/**
 * @brief 分析情感
 */
esp_err_t nlp_processor_analyze_sentiment(const char *text, float *sentiment_score)
{
    if (text == NULL || sentiment_score == NULL) {
        ai_assistant_log_error(TAG, "Invalid parameters");
        return ESP_ERR_INVALID_ARG;
    }
    
    ai_assistant_log_info(TAG, "Analyzing sentiment for: %s", text);
    
    s_nlp_state = NLP_PROCESSOR_STATE_PROCESSING;
    
    // 简单的情感分析 - 基于关键词
    const char *positive_words[] = {"好", "棒", "喜欢", "开心", "高兴", "满意", "不错", "很好", "谢谢", NULL};
    const char *negative_words[] = {"不好", "坏", "讨厌", "生气", "难过", "失望", "糟糕", "烦", "问题", NULL};
    
    int positive_count = 0;
    int negative_count = 0;
    
    // 转换为小写进行匹配
    char *text_lower = strdup(text);
    if (text_lower == NULL) {
        s_nlp_state = NLP_PROCESSOR_STATE_IDLE;
        return ESP_ERR_NO_MEM;
    }
    
    for (char *p = text_lower; *p; p++) {
        *p = tolower(*p);
    }
    
    // 统计正面词汇
    for (int i = 0; positive_words[i] != NULL; i++) {
        if (strstr(text_lower, positive_words[i]) != NULL) {
            positive_count++;
        }
    }
    
    // 统计负面词汇
    for (int i = 0; negative_words[i] != NULL; i++) {
        if (strstr(text_lower, negative_words[i]) != NULL) {
            negative_count++;
        }
    }
    
    // 计算情感分数 (-1.0 到 1.0)
    int total_words = positive_count + negative_count;
    if (total_words > 0) {
        *sentiment_score = (float)(positive_count - negative_count) / total_words;
    } else {
        *sentiment_score = 0.0f; // 中性
    }
    
    free(text_lower);
    s_nlp_state = NLP_PROCESSOR_STATE_IDLE;
    
    ai_assistant_log_info(TAG, "Sentiment score: %.2f", *sentiment_score);
    return ESP_OK;
}

/**
 * @brief 提取关键词
 */
esp_err_t nlp_processor_extract_keywords(const char *text, char keywords[][NLP_MAX_ENTITY_LEN], uint8_t *keyword_count)
{
    if (text == NULL || keywords == NULL || keyword_count == NULL) {
        ai_assistant_log_error(TAG, "Invalid parameters");
        return ESP_ERR_INVALID_ARG;
    }
    
    ai_assistant_log_info(TAG, "Extracting keywords from: %s", text);
    
    s_nlp_state = NLP_PROCESSOR_STATE_PROCESSING;
    
    *keyword_count = 0;
    
    // 分词
    char tokens[20][NLP_MAX_ENTITY_LEN];
    uint8_t token_count = 0;
    nlp_processor_tokenize(text, tokens, &token_count);
    
    // 去除停用词并提取关键词
    const char **stopwords = (s_nlp_config.language == 0) ? chinese_stopwords : english_stopwords;
    
    for (uint8_t i = 0; i < token_count && *keyword_count < 10; i++) {
        bool is_stopword = false;
        
        // 检查是否为停用词
        for (int j = 0; stopwords[j] != NULL; j++) {
            if (strcmp(tokens[i], stopwords[j]) == 0) {
                is_stopword = true;
                break;
            }
        }
        
        // 如果不是停用词且长度大于1，则作为关键词
        if (!is_stopword && strlen(tokens[i]) > 1) {
            strncpy(keywords[*keyword_count], tokens[i], NLP_MAX_ENTITY_LEN - 1);
            keywords[*keyword_count][NLP_MAX_ENTITY_LEN - 1] = '\0';
            (*keyword_count)++;
        }
    }
    
    s_nlp_state = NLP_PROCESSOR_STATE_IDLE;
    
    ai_assistant_log_info(TAG, "Extracted %d keywords", *keyword_count);
    return ESP_OK;
}

/**
 * @brief 获取NLP处理器状态
 */
nlp_processor_state_t nlp_processor_get_state(void)
{
    return s_nlp_state;
}

/**
 * @brief 加载NLP模型
 */
static esp_err_t nlp_processor_load_model(const char *model_path)
{
    ai_assistant_log_info(TAG, "Loading NLP model from: %s", model_path);
    
    // 这里应该实现实际的模型加载逻辑
    // 例如加载词向量、分类器等
    
    ai_assistant_log_info(TAG, "NLP model loading not implemented");
    return ESP_ERR_NOT_SUPPORTED;
}

/**
 * @brief 预处理文本
 */
static esp_err_t nlp_processor_preprocess_text(const char *input, char *output, size_t output_len)
{
    if (input == NULL || output == NULL || output_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 复制输入到输出
    strncpy(output, input, output_len - 1);
    output[output_len - 1] = '\0';
    
    // 转换为小写
    for (char *p = output; *p; p++) {
        *p = tolower(*p);
    }
    
    // 去除多余的空格
    char *src = output, *dst = output;
    bool space_seen = false;
    
    while (*src) {
        if (isspace(*src)) {
            if (!space_seen) {
                *dst++ = ' ';
                space_seen = true;
            }
        } else {
            *dst++ = *src;
            space_seen = false;
        }
        src++;
    }
    *dst = '\0';
    
    return ESP_OK;
}

/**
 * @brief 分词
 */
static esp_err_t nlp_processor_tokenize(const char *text, char tokens[][NLP_MAX_ENTITY_LEN], uint8_t *token_count)
{
    if (text == NULL || tokens == NULL || token_count == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *token_count = 0;
    
    char *text_copy = strdup(text);
    if (text_copy == NULL) {
        return ESP_ERR_NO_MEM;
    }
    
    // 简单的分词 - 按空格和标点符号分割
    char *token = strtok(text_copy, " \t\n\r\f\v.,!?;:");
    while (token != NULL && *token_count < 20) {
        if (strlen(token) > 0) {
            strncpy(tokens[*token_count], token, NLP_MAX_ENTITY_LEN - 1);
            tokens[*token_count][NLP_MAX_ENTITY_LEN - 1] = '\0';
            (*token_count)++;
        }
        token = strtok(NULL, " \t\n\r\f\v.,!?;:");
    }
    
    free(text_copy);
    return ESP_OK;
}

/**
 * @brief 去除停用词
 */
static esp_err_t nlp_processor_remove_stopwords(const char *text, char *output, size_t output_len)
{
    if (text == NULL || output == NULL || output_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 分词
    char tokens[20][NLP_MAX_ENTITY_LEN];
    uint8_t token_count = 0;
    nlp_processor_tokenize(text, tokens, &token_count);
    
    // 去除停用词
    const char **stopwords = (s_nlp_config.language == 0) ? chinese_stopwords : english_stopwords;
    
    output[0] = '\0';
    for (uint8_t i = 0; i < token_count; i++) {
        bool is_stopword = false;
        
        for (int j = 0; stopwords[j] != NULL; j++) {
            if (strcmp(tokens[i], stopwords[j]) == 0) {
                is_stopword = true;
                break;
            }
        }
        
        if (!is_stopword) {
            if (strlen(output) > 0) {
                strncat(output, " ", output_len - strlen(output) - 1);
            }
            strncat(output, tokens[i], output_len - strlen(output) - 1);
        }
    }
    
    return ESP_OK;
}

/**
 * @brief 词干提取
 */
static esp_err_t nlp_processor_stem_words(const char *text, char *output, size_t output_len)
{
    if (text == NULL || output == NULL || output_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 简单的词干提取 - 去除常见后缀
    strncpy(output, text, output_len - 1);
    output[output_len - 1] = '\0';
    
    // 这里可以实现更复杂的词干提取算法
    // 目前只是简单复制
    
    return ESP_OK;
}

/**
 * @brief 计算文本相似度
 */
static esp_err_t nlp_processor_calculate_similarity(const char *text1, const char *text2, float *similarity)
{
    if (text1 == NULL || text2 == NULL || similarity == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 简单的相似度计算 - 基于共同词汇数量
    char tokens1[20][NLP_MAX_ENTITY_LEN];
    char tokens2[20][NLP_MAX_ENTITY_LEN];
    uint8_t count1 = 0, count2 = 0;
    
    nlp_processor_tokenize(text1, tokens1, &count1);
    nlp_processor_tokenize(text2, tokens2, &count2);
    
    int common_words = 0;
    for (uint8_t i = 0; i < count1; i++) {
        for (uint8_t j = 0; j < count2; j++) {
            if (strcmp(tokens1[i], tokens2[j]) == 0) {
                common_words++;
                break;
            }
        }
    }
    
    int total_words = count1 + count2;
    if (total_words > 0) {
        *similarity = (float)(2 * common_words) / total_words;
    } else {
        *similarity = 0.0f;
    }
    
    return ESP_OK;
}
