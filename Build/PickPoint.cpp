#include  "PickPoint.h"


bool PickPoint(const char* prompt, API_Coord* c) {
	GSErrCode			err = NoError;
	API_GetPointType pointInfo={};

	CHTruncate(prompt, pointInfo.prompt, sizeof(pointInfo.prompt));
	pointInfo.changeFilter = false;
	pointInfo.changePlane = false;

	ACAPI_UserInput_GetPoint(&pointInfo, nullptr);
	if (err != NoError) {
		return false;
	}
	c->x = pointInfo.pos.x;
	c->y = pointInfo.pos.y;
	return true;
}