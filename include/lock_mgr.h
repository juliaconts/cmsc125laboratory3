#ifndef LOCK_MGR_H
#define LOCK_MGR_H

// handles transfers safely without deadlocking
int transfer(int from_id, int to_id, int amount_centavos);

#endif