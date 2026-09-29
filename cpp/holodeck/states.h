#ifndef STATES_H
#define STATES_H

class States
{
	public:
		class Grid
		{
			public:
				enum GridState { ON, OFF, Disable, Enable };
		};

		class World
		{	
			public:
				enum System { Initalizing, Running, Loading, Unloading, Design, CameraControl,RequestLoad, RequestSave};

			private:
				World();
		};



	private:
		States();


};
#endif