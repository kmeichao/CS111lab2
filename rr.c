#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;

  TAILQ_ENTRY(process)
  pointers;

  /* Additional fields here */
  u32 remaining_time;
  /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path,
                    struct process **process_data,
                    u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;
  u32 size;
  init_processes(argv[1], &data, &size);

  u32 quantum_length = next_int_from_c_str(argv[2]);

  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;

  /* Your code here */
  struct process *current_process;
  struct process *active_process = NULL;
  u32 active_process_time = 0;
  u32 active_process_pid = 0;

  u32 current_time = 0;
  u32 processes_left = size;

  u32 total_start_time = 0;
  u32 total_arrival_time = 0;
  u32 total_burst_time = 0;
  u32 total_complete_time = 0;

  printf("processes left: %d", size);

  while (processes_left > 0)
  {
    // select processes to be added to the queue
    for (u32 i = 0; i < size; i++)
    {
      current_process = &data[i];
      if (current_time == current_process->arrival_time)
      {
        struct process *added_node = current_process;
        added_node->remaining_time = current_process->burst_time;
        total_arrival_time += added_node->arrival_time;
        total_burst_time += added_node->burst_time;
        TAILQ_INSERT_TAIL(&list, added_node, pointers);
      }
    }

    // if there is no process active, make first process on queue active
    if (active_process_pid == 0)
    {
      if (!TAILQ_EMPTY(&list))
      {
        struct process *first = TAILQ_FIRST(&list);
        active_process = first;
        active_process_pid = active_process->pid;
        TAILQ_REMOVE(&list, first, pointers);
      }
      else
      {
        current_time++;
        continue;
      }
      active_process_time = 0;
    }

    if (active_process->remaining_time == active_process->burst_time)
    {
      total_start_time += current_time;
    }

    if (active_process != 0 && active_process_time == quantum_length)
    {
      // add the current process to the end of the queue
      TAILQ_INSERT_TAIL(&list, current_process, pointers);
      active_process_pid = 0;
    }

    active_process->remaining_time--;
    active_process_time++;
    current_time++;

    if (active_process->remaining_time == 0)
    {
      total_complete_time += current_time;
      active_process_pid = 0;
      processes_left--;
    }
  }

  total_response_time = total_start_time - total_arrival_time;
  total_waiting_time = total_complete_time - total_burst_time - total_arrival_time;

  /* End of "Your code here" */

  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  return 0;
}
