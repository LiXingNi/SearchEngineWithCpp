ʵ���˼򵥵����������з�Ϊ����ģʽ��
	һ���������д����� doc �������򣬳�Ϊ Base_algorithm
	һ������ idf �� doc �������򣬳�Ϊ idf_algorithm

���������ִַ�ģʽ��
	һ�� cutForSearch ,�÷ִʴ������ࣺ������������á�,������������������������족�������족,... ��Ϊ��Search_mode
	һ�� cut ���÷ִ�Ϊ��ȷ�ִʣ����������ࣺ������������á��������������������������á� ��Ϊ��Accurate_mode

�����ࣺ
	���ࣺ
		SearchSystem
	�����ࣺ
		SearchSystemBase, ��Ӧ Base_algorithm,�������������캯�� SearchSystemBase(ACCURATE_SEG_MODE),SearchSystemBase(Accurate_mode)
		
		SearchSystemIdf, ��Ӧ Idf_algorithm,ͬ��������������ִַ�ģʽ�Ĺ��캯��
	
	�ü̳���ϵ�еĶ���ͨ������������������
		obj.oneSearch(sentence);
	
ͳ�������ࣺ
	���ࣺ
		CountScore , ����{3,5,10}���� NDCG �����������㷨������
	�����ࣺ
		CountScoreBase, ��Ӧͳ�� Base_algorithm �㷨�����ܣ�ͬ���ṩ������ִַ�ģʽ�Ĺ��캯��
		CountScoreIdf, ��Ӧͳ�� Idf_algorithm �㷨�����ܣ�ͬ���ṩ������ִַ�ģʽ�Ĺ��캯��

	�ü̳���ϵ�еĶ���ͨ�����������ò����������ܣ�
		obj.count_score();
		 