/*--------------------------------------------------

   DoubleCherryGB - Gameboy Emulator - 4Players (based on TGBDual)
   Copyright (C) 2023  Tim Oelrichs

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

//-------------------------------------------------

#include "gb.h"
#include <vector>
#include <queue>
#include <ctime>
#include <cstdlib>
#include <fstream>

tetris_4p_hack::tetris_4p_hack(std::vector<gb*> g_gb) {

	v_gb.insert(v_gb.begin(), std::begin(g_gb), std::end(g_gb));

	mem.tetris_state = TITLE_SCREEN;

	mem.seri_occer = 4096 * 1024 * 4;
	mem.transfer_speed = 4096 * 1024 / 16;

	for (size_t i = 0; i < v_gb.size(); i++)
	{
		mem.players_state[i] = IS_ALIVE;
		//cpu* cpu = v_gb[i]->get_cpu();
	}
		
	init_send_data_queue();
}

void tetris_4p_hack::clear_data_for_next_round()
{
	for (size_t i = 0; i < v_gb.size(); i++)
	{
		mem.players_state[i] = IS_ALIVE;
		//cpu* cpu = v_gb[i]->get_cpu();
		mem.in_data_buffer[i] = 0;
		mem.next_bytes_to_send[i] = 0;
	}
}

void tetris_4p_hack::reset()
{

	mem.tetris_state = TITLE_SCREEN;

	mem.seri_occer = 4096 * 1024 * 4;
	mem.transfer_speed = 4096 * 1024 / 16;

	clear_data_for_next_round();
	mem.send_data_queue = std::queue<byte>();
	init_send_data_queue();
}


void tetris_4p_hack::init_send_data_queue() {

	//mem.send_data_queue.push(0x29);   //start connection
	mem.send_data_queue.push(0x1c);   //music a is selected 
	mem.send_data_queue.push(0x1c);   //music a is selected 
	mem.send_data_queue.push(0x1c);   //music a is selected 
	mem.send_data_queue.push(0x1c);   //music a is selected 
	mem.send_data_queue.push(0x1c);   //music a is selected 
	mem.send_data_queue.push(0x1c);   //music a is selected 
	mem.send_data_queue.push(0x1c);   //music a is selected 
	mem.send_data_queue.push(0x1c);   //music a is selected 
	mem.send_data_queue.push(0x1c);   //music a is selected 
	mem.send_data_queue.push(0x1c);   //music a is selected 
	mem.send_data_queue.push(0x1c);   //music a is selected 
	mem.send_data_queue.push(0x50);   //confirm music selection
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x0);    //height 0 is selected
	mem.send_data_queue.push(0x60);   //confirm height selection
	mem.send_data_queue.push(0x29);   //start sending height blocks
}


void tetris_4p_hack::log_traffic(byte id, byte b)
{

	std::string filePath = "./tetrishack_log.txt";
	std::ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
	if (id < 4)
	{
		ofs << "" << std::hex << (int)b << " ";

	}
	else
	{
		ofs << "" << std::hex << (int)b << std::endl;
	}

	ofs.close();

}

bool tetris_4p_hack::is_ready_to_process() {

	get_all_SC_reg_data();
	return is_expected_data(0x80) && all_IE_are_handled();
}

bool tetris_4p_hack::all_IE_are_handled() 
{
	for (int i = 0; i < v_gb.size(); i++)
	{
		if ((v_gb[i]->get_regs()->IF & v_gb[i]->get_regs()->IE & INT_SERIAL)) return false;
	}
	return true;

}

void tetris_4p_hack::process() {


	if (mem.tetris_state != WINNER_SCREEN) {

		if (v_gb[0]->get_cpu()->total_clock < mem.seri_occer) return;
		mem.seri_occer = v_gb[0]->get_cpu()->total_clock + mem.transfer_speed;
		if (!is_ready_to_process()) return;
	
	}
	
	switch (mem.tetris_state)
	{
	case TITLE_SCREEN:
	{
		
		get_all_SB_reg_data();
		if (is_expected_data(0xFF)) return; 
		get_all_SB_reg_data();
		if (is_expected_data(0x55))
		{
			broadcast_byte(0x29);
			mem.tetris_state = MUSIC_SELECT;
		}
		
	
		/*
		if (++process_counter < 5) return 0;
		process_counter = 0;
		if (!is_ready_to_process()) return 0;
		*/
		//if (!is_ready_to_process()) return;

			
		
		break;
	}
	case MUSIC_SELECT:
	{
		/*
		if (++process_counter < 5) return 0;
		process_counter = 0;
		*/
		//if (!is_ready_to_process()) return;

		if (!mem.send_data_queue.empty())
		{
			byte data = mem.send_data_queue.front();
			mem.send_data_queue.pop();
			broadcast_byte(data);
			if (mem.send_data_queue.empty())
			{
				mem.tetris_state = HEIGHT_SELECT;
				generate_height_blocks();
			}
	
		}
		break;
	}
	case HEIGHT_SELECT:
	{
		/*
		if (++process_counter < 5 && !is_ready_to_process()) return 0;
		process_counter = 0;
		*/
		mem.transfer_speed = 4096;
		
		if (!mem.out_height_blocks_queue.empty())
		{
			byte data = mem.out_height_blocks_queue.front();
			mem.out_height_blocks_queue.pop();
			broadcast_byte(data);
			if (mem.out_height_blocks_queue.empty())
			{
				generate_falling_blocks();
			}
			break;
		}
		if (!mem.out_falling_blocks_queue.empty())
		{
			byte data = mem.out_falling_blocks_queue.front();
			mem.out_falling_blocks_queue.pop();
			broadcast_byte(data);
			if (mem.out_falling_blocks_queue.empty())
			{
				mem.tetris_state = IN_GAME;
			}
			break;
		}
		break;
	}
	case IN_GAME:
	{

		mem.transfer_speed = 4096 * 1024 / 8;
		/*
		if (++process_counter < 5) return 0;
		process_counter = 0;
		if (!is_ready_to_process()) return 0;
		*/


		get_all_SC_reg_data();
		handle_ingame_data();
		break;

	}
	case WINNER_SCREEN:
	{
		
		//wait 5 Seconds
		if (++mem.process_counter < 154 * 60 * 5) return;
		mem.process_counter = 0;
	
		// would be better to handle actually answers
		mem.send_data_queue.push(0x60);   
		mem.send_data_queue.push(0x02);   
		mem.send_data_queue.push(0x02);   
		mem.send_data_queue.push(0x02);
		mem.send_data_queue.push(0x79);    
		//mem.send_data_queue.push(0x29);   //start sending height blocks

		clear_data_for_next_round();
		mem.tetris_state = START_NEXT;

		break;
	}
	case START_NEXT:
	{
		/*
		if (++process_counter < 5 && !is_ready_to_process()) return 0;
		process_counter = 0;
		*/

		if (!mem.send_data_queue.empty())
		{
			byte data = mem.send_data_queue.front();
			mem.send_data_queue.pop();
			broadcast_byte(data);
		
			break; 
		}

		
		get_all_SB_reg_data();

		if (is_expected_data(0x55))
		{
			
			mem.tetris_state = HEIGHT_SELECT;
			generate_height_blocks();
	
		}
		broadcast_byte(0x29);

		break;
	}

	default:
		break;
	}


	//seri_occer = v_gb[0]->get_cpu()->total_clock + transfer_speed;
}


void tetris_4p_hack::generate_height_blocks() 
{
	//for now create just empty blocks
	for (int i = 0; i < 100; i++)
	{
		mem.out_height_blocks_queue.push(0x2f); // 0x2f empty block
	}
	mem.out_height_blocks_queue.push(0x29); // start sending fallingblocks
};

void tetris_4p_hack::generate_falling_blocks()
{
	std::srand(std::time(0));
	int size_of_choice = sizeof(mem.falling_block_choice) / sizeof(byte);

	for (int i = 0; i < 256; ++i) 
	{
		int randomIndex = std::rand() % size_of_choice;
		mem.out_falling_blocks_queue.push(mem.falling_block_choice[randomIndex]);
	}
	mem.out_falling_blocks_queue.push(0x30);    //finished sending blocks
	mem.out_falling_blocks_queue.push(0x00);
	mem.out_falling_blocks_queue.push(0x02);
	mem.out_falling_blocks_queue.push(0x02);
	mem.out_falling_blocks_queue.push(0x20);   //start in_game
	mem.out_falling_blocks_queue.push(0x0);
	mem.out_falling_blocks_queue.push(0x0);
	mem.out_falling_blocks_queue.push(0x0);
	mem.out_falling_blocks_queue.push(0x0);
	
};

void tetris_4p_hack::get_all_SC_reg_data()
{
	for (int i = 0; i < v_gb.size(); i++)
	{
		byte data = v_gb[i]->get_regs()->SC;
		mem.in_data_buffer[i] = data;
	}
};

void tetris_4p_hack::get_all_SB_reg_data()
{
	for (int i = 0; i < v_gb.size(); i++)
	{
		byte data = v_gb[i]->get_regs()->SB;
		mem.in_data_buffer[i] = data;
	}
};

bool tetris_4p_hack::is_expected_data(byte data)
{
	for (int i = 0; i < v_gb.size(); i++)
	{
		if (mem.in_data_buffer[i] != data) return false; 
	}
	return true;
}

void tetris_4p_hack::handle_ingame_data() 
{
	get_all_SB_reg_data();
	update_ingame_states();
	send_ingame_bytes();
}

void tetris_4p_hack::send_byte(byte which, byte dat)
{
	byte ret = v_gb[which]->get_cpu()->seri_send(dat);

	if (which == 0) log_traffic(0, dat);
	log_traffic(which + 1, ret);

}

void tetris_4p_hack::broadcast_byte(byte dat)
{
	for (byte i = 0x00; i < (byte)v_gb.size(); i++)
	{
		send_byte(i, dat);
	}
}

int tetris_4p_hack::check_winner_id()
{
	int alive_count = 0;
	int last_alive_id = 0;

	for (int i = 0; i < v_gb.size(); i++)
	{
		if (mem.players_state[i] == IS_ALIVE)
		{
			alive_count++;
			last_alive_id = i + 1;
		}
	}

	if (alive_count == 1) return last_alive_id;
	if (alive_count == 0) return -1;
	return 0;
}

void tetris_4p_hack::update_ingame_states()
{
	mem.current_max_height = 0;

	for (int i = 0; i < v_gb.size(); i++)
	{
		switch (mem.players_state[i])
		{
		case IS_WINNER:
		{

			if (mem.in_data_buffer[i] == 0x34)
			{
				mem.players_state[i] = IS_LOOSER; // ;D need new states
				mem.next_bytes_to_send[i] = 0x43;
			}
			else {
				mem.next_bytes_to_send[i] = 0x34;

			}
			break; 
		}
		case IS_LOOSER:
		{
			mem.players_state[i] = IS_IN_WINNER_SCREEN;
			break; 
		}
		case IS_KO:
		{
			if (mem.in_data_buffer[i] == 0x34)
			{
				mem.players_state[i] = IS_LOOSER;
				mem.next_bytes_to_send[i] = 0x43;
			}
			else {
				mem.next_bytes_to_send[i] = 0xaa;

			}
			break;
		}
		case IS_ALIVE:
		{

			if (mem.in_data_buffer[i] == 0x44) // not ready yet
			{
				mem.next_bytes_to_send[i] = 0x20;
				break; 
			}

			if (mem.in_data_buffer[i] == 0xaa) // 0xaa is ko
			{
				mem.players_state[i] = IS_KO;
				mem.next_bytes_to_send[i] = 0xaa; // send ko for draw, cause only winning counts
				break;
			}

			if (mem.in_data_buffer[i] == 0x34)
			{
				mem.players_state[i] = IS_WINNER;
				mem.next_bytes_to_send[i] = 0x43;
				break;
			}


			if ((mem.in_data_buffer[i] & (byte)0x80) == (byte)0x80) // 0x8x send lines
			{
				if (mem.in_data_buffer[i] != (byte)0x80) {
					tetris_4p_hack_lines_packet lines;
					lines.lines = mem.in_data_buffer[i];
					lines.from = i;
					mem.lines_queue.push(lines);
				}
				break; 
			}

			mem.current_max_height = mem.current_max_height < mem.in_data_buffer[i] ? mem.in_data_buffer[i] : mem.current_max_height;
			break; 
		}

		default:
			break;
		}
	}

	if (all_are_in_winnerscreen()) mem.tetris_state = WINNER_SCREEN;

}

bool tetris_4p_hack::all_are_in_winnerscreen() {

	for (int i = 0; i < v_gb.size(); i++)
	{
		if (mem.players_state[i] != IS_IN_WINNER_SCREEN) return false;
	}
	return true; 
}

void tetris_4p_hack::send_ingame_bytes() 
{
	int winner = check_winner_id();
	if (winner && winner != -1) {
		mem.next_bytes_to_send[winner - 1] = 0xaa;
		mem.players_state[winner - 1] = IS_WINNER;
	}
	

	if (!mem.lines_queue.empty())
	{
		tetris_4p_hack_lines_packet next = mem.lines_queue.front();
		mem.lines_queue.pop();
		for (int i = 0; i < v_gb.size(); i++)
		{
			if (mem.players_state[i] == IS_IN_WINNER_SCREEN) continue;
			if (next.from == i) continue;
			if (!mem.next_bytes_to_send[i]) mem.next_bytes_to_send[i] = next.lines; // ?? do lines get lost??
		}
	}

	for (int i = 0; i < v_gb.size(); i++)
	{
		if (!mem.next_bytes_to_send[i]) mem.next_bytes_to_send[i] = mem.current_max_height;
		if (mem.players_state[i] != IS_IN_WINNER_SCREEN) 	send_byte(i, mem.next_bytes_to_send[i]);
	
		mem.next_bytes_to_send[i] = 0;
	}

}

void tetris_4p_hack::save_mem_state(void* buf) 
{
	std::stringstream ss; // any stream can be used
	cereal::BinaryOutputArchive oarchive(ss); // Create an output archive
	oarchive(this->mem); // Write the data to the archive
	
	/*
	dmg07_mem_state_size mem_size{};
	mem_size.size = ss.str().length();
	oarchive(mem_size);

*/
	serializer s(buf, serializer::SAVE_BUF);
	s.process((void*)ss.str().data(), ss.str().length());
	buf += ss.str().length();
}
void tetris_4p_hack::restore_mem_state(void* buf)
{
	std::stringstream ss2;
	ss2.write((const char*)buf, 248);

	cereal::BinaryInputArchive iarchive(ss2); // Create an input archive
	iarchive(this->mem);

}

size_t tetris_4p_hack::get_state_size() {

}