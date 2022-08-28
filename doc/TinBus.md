# Tinbus





```
# Flow Flowchart
```mermaid
flowchart TD
  A[Init] --> B[RX Idle]

  B -- Yes --> C[OK];
  C --> D[Rethink];
  D --> B;
  B -- No ----> E[End];
```
