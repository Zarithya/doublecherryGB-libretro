#include "../gb.h"

class faceball2000_cable : public link_target {

public:
	faceball2000_cable(std::vector<gb*> g_gb);
	byte seri_send(byte data) override;
	byte get_SB_value() override;
	byte get_SC_value() override;
	void reset() override;

private:
	std::vector<gb*> v_gb;

};