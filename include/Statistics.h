#if !defined(STATISTICS_H)
#define STATISTICS_H


struct ClipData
{
	ClipData() {
		position = 0;
		price = 0.0;
		profit = 0.0;
	}

	int position;
	double price;
	double profit;
};

class Statistics
{
public:
	static Statistics& Instance();

	void run();

private:
	static Statistics* m_pInstance;
};

#endif
