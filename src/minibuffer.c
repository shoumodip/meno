#include "minibuffer.h"

/*
 * Go to the next character in the minibuffer
 * @param minibuffer *Minibuffer The minibuffer to move in
 */
void minibuffer_next_char(Minibuffer *minibuffer)
{
    if (minibuffer->x < minibuffer->line.length)
        minibuffer->x++;
}

/*
 * Go to the previous character in the minibuffer
 * @param minibuffer *Minibuffer The minibuffer to move in
 */
void minibuffer_prev_char(Minibuffer *minibuffer)
{
    if (minibuffer->x > 0)
        minibuffer->x--;
}

/*
 * Go to the next word in the minibuffer
 * @param minibuffer *Minibuffer The minibuffer to move in
 */
void minibuffer_next_word(Minibuffer *minibuffer)
{
    minibuffer->x = string_next_word(minibuffer->line, minibuffer->x);
}

/*
 * Go to the previous word in the minibuffer
 * @param minibuffer *Minibuffer The minibuffer to move in
 */
void minibuffer_prev_word(Minibuffer *minibuffer)
{
    minibuffer->x = string_prev_word(minibuffer->line, minibuffer->x);
}

/*
 * Go to the start of the minibuffer
 * @param minibuffer *Minibuffer The minibuffer to move in
 */
void minibuffer_goto_head(Minibuffer *minibuffer)
{
    minibuffer->x = 0;
}

/*
 * Go to the end of the minibuffer
 * @param minibuffer *Minibuffer The minibuffer to move in
 */
void minibuffer_goto_tail(Minibuffer *minibuffer)
{
    minibuffer->x = minibuffer->line.length;
}

/*
 * Insert a character in a minibuffer
 * @param minibuffer *Minibuffer The minibuffer to move in
 * @param ch char The character to insert
 */
void minibuffer_insert_char(Minibuffer *minibuffer, char ch)
{
    if (ch != '\n')
        string_insert(&minibuffer->line, minibuffer->x++, &ch, 1);
}

/*
 * Delete the range of characters covered by a motion
 * @param minibuffer *Minibuffer The minibuffer to delete the range in
 * @param motion void (*)(Minibuffer *) The motion which creates the range
 */
void minibuffer_delete_motion(Minibuffer *minibuffer, void (*motion)(Minibuffer *))
{
    size_t origin = minibuffer->x;
    motion(minibuffer);
    minibuffer->x = string_delete_range(&minibuffer->line, origin, minibuffer->x);
}

/*
 * Update the minibuffer according based on a key
 * @param minibuffer *Minibuffer The minibuffer to update
 * @param ch int The key which decides the fate of the minibuffer
 * @return bool Whether the minibuffer has been stopped
 */
bool minibuffer_update(Minibuffer *minibuffer, int ch)
{
    switch (ch) {
    case CTRL('q'):
        string_free(&minibuffer->line);
        return false;

    case KEY_RETURN:
        return false;

    case KEY_RIGHT:
    case CTRL('f'):
        minibuffer_next_char(minibuffer);
        break;

    case KEY_LEFT:
    case CTRL('b'):
        minibuffer_prev_char(minibuffer);
        break;

    case KEY_HOME:
    case CTRL('a'):
        minibuffer_goto_head(minibuffer);
        break;

    case KEY_END:
    case CTRL('e'):
        minibuffer_goto_tail(minibuffer);
        break;

    case KEY_DC:
    case CTRL('d'):
        minibuffer_delete_motion(minibuffer, &minibuffer_next_char);
        break;

    case KEY_BACKSPACE:
    case CTRL('k'):
        minibuffer_delete_motion(minibuffer, &minibuffer_prev_char);
        break;

    case KEY_ESCAPE:
        switch (ch = getch()) {
        case 'f':
            minibuffer_next_word(minibuffer);
            break;

        case 'b':
            minibuffer_prev_word(minibuffer);
            break;

        case KEY_DC:
        case 'd':
            minibuffer_delete_motion(minibuffer, &minibuffer_next_word);
            break;

        case KEY_BACKSPACE:
        case 'k':
            minibuffer_delete_motion(minibuffer, &minibuffer_prev_word);
            break;
        }
        break;
    default:
        if (isprint(ch)) minibuffer_insert_char(minibuffer, ch);
        break;
    }

    return true;
}
