# INTRODUCTION #

## Motivation ##

I noticed about this project while reading the TEDECO web page. Then I
went to talk with Jesús Martínez Mateo, a TEDECO member, and he explained
me the problem they were facing when they tried to, remotely, impart some
classes to students at University of Ngozi.

Here a brief resume of the cause of that problem:

Nowadays inhabitants from developed countries have access to the Internet at
high speeds. However this is not the same situation at developing
countries. This, among other circumstances, is commonly called the
digital divide. One of the effects of this gap is that
software developed in developed countries does not fit in the infrastructures
of worse equipped countries.

TEDECO collaborates actively with the University of Ngozi at
Burundi, and one of the task they are performing is remote teaching.

The Internet connection at Ngozi University is done using satellite based
technology.
Their ISP gives them a very asymmetric low speed link (for the todays
standards). This connection is shared among all people at university.

This raises a problem, remote teaching, sometimes, needs of speeches
from teacher, better, if that is accompanied with some visual references such as
slides, documents or images. However most of the current software assume a high
speed Internet connection, doing them unable to work in the circumstances
described.

As an example, using Skype software floods the uplink link of the
Ngozi University connection leaving all other users without Internet access.

## Objetives ##

The project aim is to develop a solution for remote teaching, taking into
account the different circumstances exposed above.

Specifically, the objectives for the final solution are:


  * Get a way to capture and reproduce audio.
  * Find out how this audio can be transmitted: audio compression.
  * Provide a text based feedback channel.
  * Develop a network solution for audio and text transmission.
  * Produce a user interface for the above functionalities.

Those are the principal challenges to be committed, however as the project will be developed, new ones can raise.

The project intention is to produce a real, working solution, so not
strictly necessary theoretical or experimental work will be avoided.


## Preliminaries ##

This project has been approached not only to produce a working solution, but as
a starting point for future developments also. That is why the documentation
provided is intended, mainly but not only, for other developers.

This document is divided in four main sections and a series of appendices:


  * Introduction: provides a general overview of the project motivation and goals.
  * Project design: explains some design decisions taken before the project development.
  * Project development: gives a detailed vision of the project development.
  * Concluding remarks: exposes some conclusions obtained during the project development and some ideas to extend the project.