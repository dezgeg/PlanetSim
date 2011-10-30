#include <math.h>

#include "vector.h"
#include "planet.h"

static const double GAMMA = 6.674e-11;

static void update_accelerations(System* sys, int index)
{
	for(int i = 0; i < sys->nplanets; i++)
	{
		Vector force = { 0.0, 0.0, 0.0 };
		for(int j = 0; j < sys->nplanets; j++)
		{
			if(i == j)
				continue;

			// calculate force += dir(s_j -> s_i) * (GAMMA * m_i * m_j / |s_i - s_j|^2)
			// where m_x is the mass of planet x
			//       s_x is the position vector of planet x
			//       dir(x -> y) is the unit vector from point x to y, i.e. (x - y) / |x - y|
			// expands to force += (s_j - s_i) * (GAMMA * m_i * m_j / |s_i - s_j| ^ 3)
			Vector dir; // direction vector from planet j to i
			vector_sub(dir, sys->planets[j].position, sys->planets[i].position);

			Float distsq = vector_lengthsq(dir);
			vector_mul(dir, (GAMMA * sys->planets[i].mass * sys->planets[j].mass)/pow(distsq, 3.0/2.0), dir);
			vector_add(force, force, dir);
		}
		// a = F/m
		vector_mul(sys->planets[i].acceleration[index], 1.0/sys->planets[i].mass, force);
	}
}
void simulate_one_step(System* sys, int turn, Float time_step)
{
	int nplanets = sys->nplanets;
	Float dt = time_step;
	Float half_dt = 0.5 * dt;
	Float half_dt2 = 0.5 * dt * dt;
	// 1. Update x: s(t + dt) = s(t) + v(t)*dt + 1/2 a(t)*dt^2
	for(int j = 0; j < nplanets; j++)
	{
		Planet* planet = &sys->planets[j];
		Vector* pos = &planet->position;
		Vector temp;
		vector_mul(temp, dt, planet->velocity);
		vector_add(*pos, *pos, temp);
		vector_mul(temp, half_dt2, planet->acceleration[turn % 2]);
		vector_add(*pos, *pos, temp);
	}
	// 2. Calculate a(t + dt)
	update_accelerations(sys, (turn + 1) % 2);
	// 3. Calculate v(t + dt) = v(t) + 1/2 (a(t) + a(t + dt))*dt
	for(int j = 0; j < nplanets; j++)
	{
		Planet* planet = &sys->planets[j];
		Vector temp;
		vector_add(temp, planet->acceleration[0], planet->acceleration[1]);
		vector_mul(temp, half_dt, temp);
		vector_add(planet->velocity, planet->velocity, temp);
	}

}
void init_simulation(System* sys)
{
	update_accelerations(sys, 0); // set up initial accelerations
}
void simulate(FILE* outfile, System* sys)
{
	int nsteps = sys->nsteps;
	int print_period = sys->print_period;

	for(int i = 0; i < nsteps; i++)
	{
		// Print the simulation state if necessary
		if(i % print_period == 0)
		{
			sys->cur_step = i;
			print_system(outfile, sys);
		}
		simulate_one_step(sys, i, sys->time_step);
	}
}
