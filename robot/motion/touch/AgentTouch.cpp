/*
Copyright 2010 The University of New South Wales (UNSW).

This file is part of the 2010 team rUNSWift RoboCup entry. You may
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version as
modified below. As the original licensors, we add the following
conditions to that license:

In paragraph 2.b), the phrase "distribute or publish" should be
interpreted to include entry into a competition, and hence the source
of any derived work entered into a competition must be made available
to all parties involved in that competition under the terms of this
license.

In addition, if the authors of a derived work publish any conference
proceedings, journal articles or other academic papers describing that
derived work, then appropriate academic citations to the original work
must be included in that publication.

This rUNSWift source is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this source code; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "motion/touch/AgentTouch.hpp"
#include <sys/mman.h>        /* For shared memory */
#include <fcntl.h>           /* For O_* constants */
#include <cstdlib>
#include <stdexcept>
#include "utils/Logger.hpp"
#include "gamecontroller/RoboCupGameControlData.hpp"

AgentTouch::AgentTouch(int team, int player_number) {
   // open sempahore
   std::string sem_path(AGENT_SEMAPHORE);
#ifdef SIMULATION
   // If we're running a simulator build, modify the semaphore path so we don't 
   // use the same semaphore as another instance of the sim build running at
   // the same time
   int mod = (team * MAX_NUM_PLAYERS) + player_number;
   std::stringstream ss;
   ss << mod;
   sem_path += ss.str();
#endif 

   semaphore = sem_open(sem_path.c_str(), O_CREAT | O_RDWR, 0600, 0);
   if (semaphore == SEM_FAILED)
   {
      std::cerr << "sem_open() failed!\n";
      throw std::runtime_error("AgentTouch: sem_open() failed");
   }

   // open shared memory as RW
   std::string mem_path(AGENT_MEMORY);
#ifdef SIMULATION
   // Modify the shared memory path so we don't conflict with another instance.
   mem_path += ss.str();
#endif

   shared_fd = shm_open(mem_path.c_str(), O_CREAT | O_RDWR, 0600);
   if (shared_fd <= 0)
   {
      throw std::runtime_error("AgentTouch: shm_open() failed");
   }

   // map shared memory to process memory
   shared_data = (AgentData*) mmap(NULL, sizeof(AgentData),
                                   PROT_READ | PROT_WRITE,
                                   MAP_SHARED, shared_fd, 0);
   if (shared_data == MAP_FAILED)
   {
      throw std::runtime_error("AgentTouch: mmap() failed");
   }

   llog(INFO) << "AgentTouch constructed" << std::endl;
}

AgentTouch::~AgentTouch() {
   if (shared_data != MAP_FAILED) munmap(shared_data, sizeof(AgentData));
   if (shared_fd >= 0) close(shared_fd);
   if (semaphore != SEM_FAILED) sem_close(semaphore);
   llog(INFO) << "AgentTouch destroyed" << std::endl;
}

SensorValues AgentTouch::getSensors(Kinematics &kinematics) {
   while(sem_wait(semaphore) < 0);
   shared_data->sensors_read = shared_data->sensors_latest;
   return shared_data->sensors[shared_data->sensors_read];
}

bool AgentTouch::getStanding() {
   return shared_data->standing;
}

ButtonPresses AgentTouch::getButtons() {
   return shared_data->buttons[shared_data->sensors_read];
}
