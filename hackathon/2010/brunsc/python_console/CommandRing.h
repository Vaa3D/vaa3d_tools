#ifndef V3D_COMMAND_RING_H_
#define V3D_COMMAND_RING_H_

/*
 * CommandRing.h
 *
 *  Created on: Dec 24, 2010
 *      Author: cmbruns
 */

class CommandRing
{
public:
    CommandRing(int ringSize)
        : commands(ringSize),
          newestCommandIndex(-1),
          oldestCommandIndex(-1),
          currentCommandIndex(-1),
          storedProvisionalCommand("")
    {}

    bool addHistory(const QString& command) {
        if (command.length() == 0)
            return false; // Don't store empty commands
        if (newestCommandIndex == -1) {
            // This is the first command ever
            newestCommandIndex = 0;
            oldestCommandIndex = 0;
            currentCommandIndex = -1;
            commands[newestCommandIndex] = command;
            return true;
        }
        QString previousCommand = commands[newestCommandIndex];
        if (previousCommand == command) {
            currentCommandIndex = -1;
            return false; // Don't store repeated commands
	}

        increment(newestCommandIndex);
        if (oldestCommandIndex == newestCommandIndex)
            increment(oldestCommandIndex);
        currentCommandIndex = -1; // one past latest
        commands[newestCommandIndex] = command;
        return true;
    }

    QString getNextCommand(const QString& provisionalCommand)
    {
        if (newestCommandIndex == -1) // no commands yet
            return provisionalCommand;
        if (currentCommandIndex == -1) // one past newest
            return provisionalCommand;
        if (currentCommandIndex == newestCommandIndex) {
            currentCommandIndex = -1;
            return storedProvisionalCommand;
        }
        increment(currentCommandIndex);
        return commands[currentCommandIndex];
    }

    QString getPreviousCommand(const QString& provisionalCommand)
    {
        if (newestCommandIndex == -1) // no commands yet
            return provisionalCommand;
        if (currentCommandIndex == -1) { // one past newest
            currentCommandIndex = newestCommandIndex;
            storedProvisionalCommand = provisionalCommand;
            return commands[currentCommandIndex];
        }
        // Sorry, you cannot get the very oldest command.
        if (currentCommandIndex == oldestCommandIndex)
            return provisionalCommand;
        decrement(currentCommandIndex);
        return commands[currentCommandIndex];
    }

private:
    void increment(int& val) { // circular increment
        val = (val+1) % commands.size();
    }

    void decrement(int& val) { // circular decrement
        val = (val+commands.size()-1) % commands.size();
    }

    std::vector<QString> commands;
    int newestCommandIndex;
    int oldestCommandIndex;
    int currentCommandIndex;
    QString storedProvisionalCommand;
};

#endif // V3D_COMMAND_RING_H_
