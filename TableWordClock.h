/* In order to not save 16x16 for each pattern the data is compressed
 * in the following structure
 */
typedef struct {
  uint8_t displayRow;    /* which row should by displayed */
  uint16_t pattern;      /* pattern in this row */
} tableClockWordPattern_t;

typedef struct {
  uint8_t from;
  uint8_t to;
  uint8_t row;
} minutesToWordPatternMapping_t;

typedef struct {
  uint8_t hour;
  uint8_t row;
} hoursToWordPatternMapping_t;
