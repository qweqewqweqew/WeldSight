-- 创建触发器，保留最近7天的数据
CREATE TRIGGER IF NOT EXISTS auto_clean_welding_results
AFTER INSERT ON welding_results
BEGIN
    DELETE FROM welding_results 
    WHERE create_time < datetime('now', '-7 days', 'localtime');
END;