#include <functional>

using namespace std;

namespace camerautils
{
class Camera
{
private:
	/* data */
public:
	Camera(/* args */);
	void Init();
	void DeInit();
	void Snap(uint8_t *, uint32_t);
	void SnapContinuous(uint8_t *, uint32_t, std::function<void()>);
	std::string getName();
	~Camera();
};
}